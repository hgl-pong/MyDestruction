#include "FVoronoi3D.h"
#include "Utils.h"
FVoronoi3D::FVoronoi3D(BBox& boundingBox, float SquaredDistSkipPtThreshold /*= 0.0f*/)
	:NumSites(0),
	Container(nullptr)
{
	SetBBox(boundingBox);
	Container = _StandardVoroContainerInit(100, SquaredDistSkipPtThreshold);
}

FVoronoi3D::~FVoronoi3D()
{
	FDELETE(Container);
}

void FVoronoi3D::Clear()
{
	NumSites = 0;
	Cells.clear();
	Container->clear();
}

void FVoronoi3D::SetBBox(BBox& boundingBox)
{
	Bounds.Max = boundingBox.Max;
	Bounds.Min = boundingBox.Min;
}

void FVoronoi3D::AddSites(const std::vector< FVec3>& Sites, float SquaredDistSkipPtThreshold /*= 0.0f*/)
{
	int32_t OrigSitesNum = NumSites;
	if (SquaredDistSkipPtThreshold > 0)
	{
		_PutSitesWithDistanceCheck(Container, Sites, OrigSitesNum, SquaredDistSkipPtThreshold);
	}
	else
	{
		_PutSites(Container, Sites, OrigSitesNum);
	}
	NumSites += Sites.size();
}

void FVoronoi3D::AddSites(int count, float SquaredDistSkipPtThreshold /*= 0.0f*/)
{
	for (int i = 0; i < count; i++) {
		FVec3 p(RandomNumber(Bounds.Min.X, Bounds.Max.X), RandomNumber(Bounds.Min.Y, Bounds.Max.Y), RandomNumber(Bounds.Min.Z, Bounds.Max.Z));
		AddSite(p, SquaredDistSkipPtThreshold);
	}
}

void FVoronoi3D::ComputeAllCells()
{
	voro::voro_compute<voro::container> VoroCompute = Container->make_compute();

	FILE* f1 = voro::safe_fopen("neighbors_m.pov", "w");
	Cells.resize(NumSites);

	voro::c_loop_all CellIterator(*Container);
	voro::voronoicell_neighbor cell;

	if (CellIterator.start())
	{
		do
		{
			bool bCouldComputeCell = Container->compute_cell(cell, CellIterator,VoroCompute);
			if (bCouldComputeCell)
			{
				int32_t id = CellIterator.pid();
				double x, y, z;
				CellIterator.pos(x, y, z);

				VoronoiCellInfo& Cell = Cells[id];
				FVec3 pos(x, y, z);
				std::vector<FVec3>normals;
				cell.extractCellInfo(pos, Cell.Vertices, Cell.Faces, Cell.Neighbors,Cell.Normals);
				Cell.position = { (float)x,(float)y,(float)z };

				/*cell.neighbors(Cell.Neighbors);*/

				cell.draw_pov_mesh(x * 2, y * 2, z * 2, f1);
			}
		} while (CellIterator.inc());
	}

	fclose(f1);
	Container->draw_cells_gnuplot("random_points_v.gnu");
}

void FVoronoi3D::ComputeCellEdgesSerial()
{
	voro::voro_compute<voro::container> VoroCompute = Container->make_compute();

	FILE* f1 = voro::safe_fopen("neighbors_m.pov", "w");
	Cells.resize(NumSites);

	voro::c_loop_all CellIterator(*Container);
	voro::voronoicell cell;

	if (CellIterator.start())
	{
		do
		{
			bool bCouldComputeCell = Container->compute_cell(cell, CellIterator, VoroCompute);
			if (bCouldComputeCell)
			{
				int32_t id = CellIterator.pid();
				double x, y, z;
				CellIterator.pos(x, y, z);

				VoronoiCellInfo& Cell = Cells[id];
				//FVec3 pos(x * 2, y * 2, z * 2);
				FVec3 pos(x , y , z );
				std::vector<FVec3>normals;
				cell.extractCellInfo(pos, Cell.Vertices, Cell.Faces,true);

				Cell.position = { x,y,z };
				Cell.Edges.clear();
				uint32_t FaceOffset = 0;
				for (size_t ii = 0, ni = Cell.Faces.size(); ii < ni; ii += Cell.Faces[ii] + 1)
				{
					uint32_t VertCount = Cell.Faces[ii];
					uint32_t PreviousVertexIndex = Cell.Faces[FaceOffset + VertCount];
					for (uint32_t kk = 0; kk < VertCount; ++kk)
					{
						uint32_t VertexIndex = Cell.Faces[1 + FaceOffset + kk]; // Index of vertex X coordinate in raw coordinates array

						Cell.Edges.push_back({ PreviousVertexIndex, VertexIndex });
						PreviousVertexIndex = VertexIndex;
					}
					FaceOffset += VertCount + 1;
				}

				//cell.neighbors(Cell.Neighbors);

				cell.draw_pov_mesh(x * 2, y * 2, z * 2, f1);

			}
		} while (CellIterator.inc());
	}

	fclose(f1);
	Container->draw_cells_gnuplot("random_points_v.gnu");
}

void FVoronoi3D::ComputeCellEdges()
{
	voro::voro_compute<voro::container> VoroCompute = Container->make_compute();

	FILE* f1 = voro::safe_fopen("neighbors_m.pov", "w");
	Cells.resize(NumSites);

	voro::c_loop_all CellIterator(*Container);
	voro::voronoicell cell;

	if (CellIterator.start())
	{
		do
		{
			bool bCouldComputeCell = Container->compute_cell(cell, CellIterator,VoroCompute);
			if (bCouldComputeCell)
			{
				int32_t id = CellIterator.pid();
				double x, y, z;
				CellIterator.pos(x, y, z);

				VoronoiCellInfo& Cell = Cells[id];
				FVec3 pos(x, y, z);
				std::vector<FVec3>normals;
				cell.extractCellInfo(pos, Cell.Vertices, Cell.Faces);

				Cell.position = { x,y,z };
				Cell.Edges.clear();
				for (int i = 0; i<Cell.Faces.size()/3;  i++)
				{
					Cell.Edges.push_back({ Cell.Faces[3 * i],Cell.Faces[3 * i+1] });
					Cell.Edges.push_back({ Cell.Faces[3 * i],Cell.Faces[3 * i+2] });
					Cell.Edges.push_back({ Cell.Faces[3 * i+1],Cell.Faces[3 * i+2] });
				}

				//cell.neighbors(Cell.Neighbors);

				cell.draw_pov_mesh(x * 2, y * 2, z * 2, f1);

			}
		} while (CellIterator.inc());
	}

	fclose(f1);
	Container->draw_cells_gnuplot("random_points_v.gnu");
}

bool FVoronoi3D::VoronoiNeighbors(std::vector<std::vector<int>>& Neighbors, bool bExcludeBounds /*= true*/, float SquaredDistSkipPtThreshold /*= KINDA_SMALL_NUMBER*/)
{
	Neighbors.clear();
	Neighbors.resize(NumSites);

	FILE* f1 = voro::safe_fopen("neighbors_m.pov", "w");

	voro::c_loop_all CellIterator(*Container);
	voro::voronoicell_neighbor cell;
	if (CellIterator.start())
	{
		do
		{
			bool bCouldComputeCell = Container->compute_cell(cell, CellIterator);
			if (bCouldComputeCell)
			{
				int id = CellIterator.pid();

				cell.neighbors(Neighbors[id]);

				for (unsigned int j = 0; j < Neighbors[id].size(); j++) printf(" %d", Neighbors[id][j]);
				printf("\n");

				double X, Y, Z;
				CellIterator.pos(X, Y, Z);
				cell.draw_pov_mesh(X * 2, Y * 2, Z * 2, f1);
			}
		} while (CellIterator.inc());
	}
	fclose(f1);
	Container->draw_cells_gnuplot("random_points_v.gnu");
	return true;
}

bool FVoronoi3D::GetVoronoiEdges(const std::vector< FVec3>& Sites, const BBox& Bounds, std::vector<Edge>& Edges, std::vector<int32_t>& CellMember, float SquaredDistSkipPtThreshold /*= KINDA_SMALL_NUMBER*/)
{
	int32_t NumSites = Sites.size();
	/*BBox BoundingBox;
	BoundingBox.Max = Bounds.Max;
	BoundingBox.Min = Bounds.Min;*/
	ComputeCellEdges();
	return true;
}

void FVoronoi3D::BoxSampling(BBox& box, std::vector<int32_t>& CellMember, bool random)
{

}

void FVoronoi3D::SphereSampling(FVec3& center, float radius, std::vector<int32_t>& CellMember, bool random)
{

}

VoronoiCellInfo* FVoronoi3D::GetAllCells() const
{
	return const_cast<VoronoiCellInfo*>(Cells.data());
}

bool FVoronoi3D::_OutOfBox(const FVec3& p)
{
	if ((p.X > Bounds.Min.X && p.X < Bounds.Max.X) &&
		(p.Y > Bounds.Min.Y && p.Y < Bounds.Max.Y) &&
		(p.Z > Bounds.Min.Z && p.Z < Bounds.Max.Z))
		return false;
	else
		return true;
}

void FVoronoi3D::_PutSites(voro::container* Container, const std::vector< FVec3>& Sites, int32_t Offset)
{
	for (int i = 0; i < Sites.size(); i++)
	{
		const FVec3& V = Sites[i];
		if (_OutOfBox(V))
		{
			continue;
		}
		else
		{
			Container->put(Offset + i, V.X, V.Y, V.Z);
		}
	}
}

int32_t FVoronoi3D::_PutSitesWithDistanceCheck(voro::container* Container, const std::vector< FVec3>& Sites, int32_t Offset, float SquaredDistThreshold /*= 1e-4*/)
{
	int32_t SkippedPts = 0;
	for (int i = 0; i < Sites.size(); i++)
	{
		const FVec3& V = Sites[i];
		if (_OutOfBox(V))
		{
			SkippedPts++;
			continue;
		}
		else
		{
			double EX, EY, EZ;
			int ExistingPtID;
			if (Container->find_voronoi_cell(V.X, V.Y, V.Z, EX, EY, EZ, ExistingPtID))
			{
				float dx = V.X - EX;
				float dy = V.Y - EY;
				float dz = V.Z - EZ;
				if (dx * dx + dy * dy + dz * dz < SquaredDistThreshold)
				{
					SkippedPts++;
					continue;
				}
			}
			Container->put(Offset + i, V.X, V.Y, V.Z);
		}
	}
	return SkippedPts;
}

voro::container* FVoronoi3D::_StandardVoroContainerInit(int SiteCount, float SquaredDistSkipPtThreshold /*= 0.0f*/)
{
	int GridCellsX, GridCellsY, GridCellsZ;
	voro::guess_optimal(SiteCount, Bounds.Max.X - Bounds.Min.X, Bounds.Max.Y - Bounds.Min.Y, Bounds.Max.Z - Bounds.Min.Z, GridCellsX, GridCellsY, GridCellsZ);

	voro::container* Container = new voro::container(
		Bounds.Min.X, Bounds.Max.X, Bounds.Min.Y,
		Bounds.Max.Y, Bounds.Min.Z, Bounds.Max.Z,
		GridCellsX, GridCellsY, GridCellsZ, false, false, false, 10);

	return Container;
	return nullptr;
}

void FVoronoi3D::AddSite(const FVec3& Site, float SquaredDistSkipPtThreshold /*= 0.0f*/)
{
	if (_OutOfBox(Site))
	{
		return;
	}
	else
	{
		double EX, EY, EZ;
		int ExistingPtID;
		if (Container->find_voronoi_cell(Site.X, Site.Y, Site.Z, EX, EY, EZ, ExistingPtID))
		{
			float dx = Site.X - EX;
			float dy = Site.Y - EY;
			float dz = Site.Z - EZ;
			if (dx * dx + dy * dy + dz * dz < SquaredDistSkipPtThreshold)
			{
				return;
			}
		}
		Container->put(NumSites, Site.X, Site.Y, Site.Z);
		NumSites++;
	}
}
