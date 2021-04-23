#include <stdio.h>
#include <iostream>
#include <d3dx9math.h>
using namespace std;

// 1번 문제
void MatrixMul()
{
	D3DXMATRIX a(-2, 0, 3, 0, 4, 1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	D3DXMATRIX b(2, -1, 0, 0, 0, 6, 0, 0, 2, -3, 0, 0, 0, 0, 0, 0);
	D3DXMATRIX result;

	D3DXMatrixMultiply(&result, &a, &b);

	printf("| %3.0f %3.0f %3.0f %3.0f |\n", result._11, result._12, result._13, result._14);
	printf("| %3.0f %3.0f %3.0f %3.0f |\n", result._21, result._22, result._23, result._24);
	printf("| %3.0f %3.0f %3.0f %3.0f |\n", result._31, result._32, result._33, result._34);
	printf("| %3.0f %3.0f %3.0f %3.0f |\n", result._41, result._42, result._43, result._44);

}
// 2번 문제
void linearcombination()
{
	D3DXVECTOR4 u(3, 2, -1, 5);
	D3DXMATRIX a(2, -4, 0, 0, 1, 4, 0, 0, -2, -3, 0, 0, 1, 1, 0, 0);
	D3DXVECTOR4 result;

	D3DXVec4Transform(&result, &u, &a);

	printf("| %3.0f %3.0f %3.0f %3.0f |\n", result.x, result.y, result.z, result.w);

}


// 3번 문제
void Parallelogram()
{
	D3DXVECTOR3 u(3, 0, 0);
	D3DXVECTOR3 v(1, 1, 0);
	D3DXVECTOR3 result;
	
	D3DXVec3Cross(&result, &u, &v);
	cout << D3DXVec3Length(&result) << endl;
	

}

// 4번 문제
void InverseMatrix()
{
	D3DXMATRIX a(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 12, -10, 0, 1);
	D3DXMATRIX Ia;
	D3DXMatrixInverse(&Ia, 0, &a);

	printf("| %3.0f %3.0f %3.0f %3.0f |\n", Ia._11, Ia._12, Ia._13, Ia._14);
	printf("| %3.0f %3.0f %3.0f %3.0f |\n", Ia._21, Ia._22, Ia._23, Ia._24);
	printf("| %3.0f %3.0f %3.0f %3.0f |\n", Ia._31, Ia._32, Ia._33, Ia._34);
	printf("| %3.0f %3.0f %3.0f %3.0f |\n", Ia._41, Ia._42, Ia._43, Ia._44);

}

// 5번문제
void Crossrightangle()
{
	D3DXVECTOR3 v(5, 4, 1);
	D3DXVECTOR3 u;
	bool result = false;

	for (int i = 0; i < 10; i++)
	{
		u.x = i;
		for (int j = 0; j < 10; j++)
		{
			u.y = j;
			for (int k = 0; k < 10; k++)
			{
				u.z = k;
				if (D3DXVec3Dot(&v, &u) == 0)
				{
					result = true;
					break;
				}
					
			}
			if (result)
				break;
		}
		if (result)
			break;
	}

	/*while (1)
	{
		u.x = rand() % 10;
		u.y = rand() % 10;
		u.z = rand() % 10;
		if (D3DXVec3Dot(&v, &u) == 0)
			break;
	}*/

	printf("| %3.0f %3.0f %3.0f |\n", u.x, u.y, u.z);

}

void main()
{
	MatrixMul();
	cout << endl;

	linearcombination();
	cout << endl;

	Parallelogram();
	cout << endl;

	InverseMatrix();
	cout << endl;

	Crossrightangle();
}