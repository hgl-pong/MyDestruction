#include "FBoundingBox.h"
#include <algorithm>

FBoundingBox::FBoundingBox(const std::vector<FVec3>& points) {
	m_Min = { FLOAT_MAX,FLOAT_MAX,FLOAT_MAX };
	m_Max = { FLOAT_MIN,FLOAT_MIN,FLOAT_MIN};
	
	for (auto point : points) {
		Include(point);
	}
}

FBoundingBox::FBoundingBox(const std::vector<FVertex>& points) {
	m_Min = { FLOAT_MAX,FLOAT_MAX,FLOAT_MAX };
	m_Max = { FLOAT_MIN,FLOAT_MIN,FLOAT_MIN };
	for (auto point : points) {
		Include(point.position);
	}
}

FBoundingBox::FBoundingBox(FVec3& v0, FVec3& v1, FVec3& v2) {
	m_Min = { FLOAT_MAX,FLOAT_MAX,FLOAT_MAX };
	m_Max = { FLOAT_MIN,FLOAT_MIN,FLOAT_MIN };

	Include(v0);
	Include(v1);
	Include(v2);
}

FBoundingBox::FBoundingBox()
{
	m_Min = { FLOAT_MAX,FLOAT_MAX,FLOAT_MAX };
	m_Max = { FLOAT_MIN,FLOAT_MIN,FLOAT_MIN };

}

FBoundingBox::~FBoundingBox() {
	
}

bool FBoundingBox::Intersection(FBoundingBox& box) {
	FVec3 Max = m_Max.Ceiling(box.m_Max);
	FVec3 Min = m_Min.Floor(box.m_Min);
	
	FVec3 size = Max - Min;
	FVec3 sizeAB = m_Size + box.m_Size;

	return size.X<sizeAB.X&& size.Y < sizeAB.Y&& size.Z < sizeAB.Z;
}

void FBoundingBox::Merge(FBoundingBox& box) {
	m_Max = m_Max.Ceiling(box.m_Max);
	m_Min = m_Min.Floor(box.m_Min);

	m_Size = m_Max - m_Min;
}

void FBoundingBox::Include(FVec3& position)
{
	m_Max = m_Max.Ceiling(position);
	m_Min = m_Min.Floor(position);

	m_Size = m_Max - m_Min;
}
