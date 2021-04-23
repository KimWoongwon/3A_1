#include <iostream>
#include <d3dx9math.h>
using namespace std;

void main()
{
	D3DXVECTOR3 pos_1(1.0f, 1.0f, 1.0f);
	D3DXVECTOR3 pos_2(5.0f, 4.0f, 3.0f);

	// ����
	FLOAT dot_result = D3DXVec3Dot(&pos_1, &pos_2);
	cout << "dot: " << dot_result << endl;

	// ����
	D3DXVECTOR3 vec_result;
	D3DXVec3Cross(&vec_result ,&pos_1, &pos_2);
	cout << "cross: " << vec_result.x << " ," << vec_result.y << " ," << vec_result.z << endl;

	// �� ���ͻ����� ����
	FLOAT len_1 = D3DXVec3Length(&pos_1);
	FLOAT len_2 = D3DXVec3Length(&pos_2);

	FLOAT cos_dot = dot_result / (len_1 * len_2);
	cout << D3DXToDegree(acosf(cos_dot)) << endl;

	// ������ ����
	cout << D3DXVec3Length(&pos_2) << endl;
}