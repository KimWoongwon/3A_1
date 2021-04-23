#include <iostream>
using namespace std;

#include <d3dx9math.h>

void printmatrix(D3DXMATRIX _mat)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			printf("%5.2f", _mat(i, j));
		}
		cout << endl;
	}
	cout << endl;
}
int main()
{
	D3DXMATRIX mat;

	cout << "=================== Matrix Identity ===================" << endl;
	D3DXMatrixIdentity(&mat);
	printmatrix(mat);

	cout << "=================== Matrix Translation ===================" << endl;
	D3DXMatrixTranslation(&mat, 0.2f, 0.3f, 0.4f);
	printmatrix(mat);

	cout << "=================== Matrix Scaling ===================" << endl;
	D3DXMatrixScaling(&mat, 1.0f, 2.0f, 3.0f);
	printmatrix(mat);

	cout << "=================== Matrix YawPitchRoll ===================" << endl;
	D3DXMatrixRotationYawPitchRoll(&mat, D3DXToRadian(10), D3DXToRadian(20), D3DXToRadian(30));
	printmatrix(mat);

	cout << "=================== Matrix RotationAxis ===================" << endl;
	D3DXVECTOR3 v = D3DXVECTOR3(1.0f, 2.0f, 3.0f);
	D3DXMatrixRotationAxis(&mat, &v, D3DXToRadian(30));
	printmatrix(mat);

	cout << "=================== Quaternion RotationAxis ===================" << endl;
	D3DXQUATERNION q1;
	D3DXQuaternionIdentity(&q1);
	D3DXQuaternionRotationAxis(&q1, &v, D3DXToRadian(30));
	D3DXMatrixRotationQuaternion(&mat, &q1);
	printmatrix(mat);
	return 0;

	



}