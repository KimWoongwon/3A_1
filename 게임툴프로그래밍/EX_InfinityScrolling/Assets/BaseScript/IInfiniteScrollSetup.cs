using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace Du3Project
{
	interface IInfiniteScrollSetup
    {
        // ui Init 할시 처음 실행시 호출될 함수
        void OnPostSetupItems();
        // ui 업데이트용
        void OnUpdateItem(int itemCount, GameObject obj);
        // ui 강제 업데이트 세팅하면서 업데이트 호출이 안되었을시 호출하기 위함
        void DirectUpdate(GameObject obj);
    }
	
}