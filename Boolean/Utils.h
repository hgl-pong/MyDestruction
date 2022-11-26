#ifndef UTILS_H
#define UTILS_H

#define FASSERT(X)\
if(!X)\
{\
  goto Exit0;\
}

#define FDELETE(X)\
if(X)\
{\
  delete X;\
	X = nullptr; \
}

#define FDELETE_ARRAY(X)\
if(X)\
{\
  delete []X;\
	X = nullptr; \
}

#define PHYSX_RELEASE(X)\
if(X)\
{\
  X->release();\
	X = nullptr; \
}

#define FRELEASE(X)\
if(X)\
{\
  X->Release();\
	X = nullptr; \
}


#endif // UTILS_H
