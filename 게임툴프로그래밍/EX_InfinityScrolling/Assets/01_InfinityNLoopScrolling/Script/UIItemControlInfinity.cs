using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Du3Project
{
    [RequireComponent(typeof(InfiniteScroll))]
	public class UIItemControlInfinity : MonoBehaviour, IInfiniteScrollSetup
    {
        // loop방식 설정
        public bool m_ISLoop = false;
        public int LoopMaxCount = 10;

        public void OnPostSetupItems()
        {
            GetComponent<InfiniteScroll>().onUpdateItem.AddListener(OnUpdateItem);
            // 무한 반복이기때문에 Unrestricted 바꾸도록 하기
            GetComponentInParent<ScrollRect>().movementType = ScrollRect.MovementType.Unrestricted;

            if(GetComponent<InfiniteScroll>().DirectionVal == InfiniteScroll.Direction.Vertical )
            {

                GameObject.Destroy(GetComponentInParent<ScrollRect>().verticalScrollbar.gameObject);
                GetComponentInParent<ScrollRect>().verticalScrollbar = null;
                GetComponentInParent<ScrollRect>().GraphicUpdateComplete();
                GetComponentInParent<ScrollRect>().Rebuild(CanvasUpdate.PreRender);
            }
            else
            {
                GameObject.Destroy(GetComponentInParent<ScrollRect>().horizontalScrollbar.gameObject);
                GetComponentInParent<ScrollRect>().horizontalScrollbar = null;
                GetComponentInParent<ScrollRect>().GraphicUpdateComplete();
                GetComponentInParent<ScrollRect>().Rebuild(CanvasUpdate.PreRender);
            }
            
            

        }

        public void OnUpdateItem(int itemCount, GameObject obj)
        {
            UIItemElement uiitem = obj.GetComponent<UIItemElement>();

            if(m_ISLoop)
            {
                uiitem.UpdateItem( ((itemCount % LoopMaxCount) + LoopMaxCount ) % LoopMaxCount );
            }
            else
                uiitem.UpdateItem(itemCount );

        }

        public void DirectUpdate(GameObject obj)
        {
        }

        void Awake()
        {
            
        }

        void Start()
		{
			
		}

		//void Update()
		//{
			
		//}
	}
}