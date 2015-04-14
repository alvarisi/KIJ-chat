#include <stdio.h>
#include <stdlib.h>

typedef struct Primitive_Root_t{
	int n;
	int root;
	Primitive_Root_t(int _n, int _root){
		n = _n;
		root = _root;	
	}
} Primitive_Root_t;

int main()
{
	Primitive_Root_t primitiveRoot[100];
	//primitiveRoot = Primitive_Root_t(n,root)
	primitiveRoot[0] = Primitive_Root_t(3,2);
	primitiveRoot[1] = Primitive_Root_t(5,2);	
	primitiveRoot[2] = Primitive_Root_t(7,3);	
	primitiveRoot[3] = Primitive_Root_t(9,2);
	primitiveRoot[4] = Primitive_Root_t(11,2);	
	primitiveRoot[5] = Primitive_Root_t(13,6);	
	primitiveRoot[6] = Primitive_Root_t(16,5);	
	primitiveRoot[7] = Primitive_Root_t(17,10);	
	primitiveRoot[8] = Primitive_Root_t(19,10);	
	primitiveRoot[9] = Primitive_Root_t(23,10);	
	primitiveRoot[10] = Primitive_Root_t(25,2);	
	primitiveRoot[11] = Primitive_Root_t(27,2);	
	primitiveRoot[12] = Primitive_Root_t(29,10);	
	primitiveRoot[13] = Primitive_Root_t(31,17);	
	primitiveRoot[14] = Primitive_Root_t(32,5);	
	primitiveRoot[15] = Primitive_Root_t(37,5);	
	primitiveRoot[16] = Primitive_Root_t(41,6);	
	primitiveRoot[17] = Primitive_Root_t(43,28);	
	primitiveRoot[18] = Primitive_Root_t(47,10);	
	primitiveRoot[19] = Primitive_Root_t(49,10);	
	primitiveRoot[20] = Primitive_Root_t(53,26);	
	primitiveRoot[21] = Primitive_Root_t(59,10);	
	primitiveRoot[22] = Primitive_Root_t(61,10);	
	primitiveRoot[23] = Primitive_Root_t(64,5);	
	primitiveRoot[24] = Primitive_Root_t(67,12);	
	primitiveRoot[25] = Primitive_Root_t(71,62);	
	primitiveRoot[26] = Primitive_Root_t(73,5);	
	primitiveRoot[27] = Primitive_Root_t(79,29);	
	primitiveRoot[28] = Primitive_Root_t(81,11);	
	primitiveRoot[29] = Primitive_Root_t(83,50);	
	primitiveRoot[30] = Primitive_Root_t(89,30);
	primitiveRoot[31] = Primitive_Root_t(97,10);
	return 0;
}
