using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections.Generic;
using System.Linq;

namespace Du3Project
{
    // UIBehaviour
    // https://github.com/tenpn/unity3d-ui/blob/master/UnityEngine.UI/EventSystem/UIBehaviour.cs
    // https://github.com/tsubaki/Unity_UI_Samples/tree/master/Assets/InfiniteScroll
    public class InfiniteScroll : UIBehaviour
    {
        public enum Direction
        {
            Vertical,
            Horizontal,
        }

        [System.Serializable]
        public class OnItemPositionChange : UnityEngine.Events.UnityEvent<int, GameObject> { }


        // 복제용 아이템
        // ********** 복제용 아이템의 Anchors 의 값은 0, 0 왼쪽 상단을 기준으로 잡아야지됨 **********
        [SerializeField]
        private RectTransform itemPrototype;

        // 초기화에서 사용할 아이템 갯수 화면에서 보여줄것 이상의 갯수를 적용하도록 하기
        [SerializeField, Range(0, 30)]
        int instantateItemCount = 9;

        // 방향성 지정
        [SerializeField]
        private Direction m_Direction;
        public Du3Project.InfiniteScroll.Direction DirectionVal
        {
            get { return m_Direction; }
            set { m_Direction = value; }
        }
        // 이벤트용, 인스팩터에 보이기 위한 기본
        public OnItemPositionChange onUpdateItem = new OnItemPositionChange();

        // instantateItemCount 에서 만들어진 갯수들 들고있을 오브젝트값들
        [System.NonSerialized]
        public LinkedList<RectTransform> itemList = new LinkedList<RectTransform>();

        // 위치값용
        protected float diffPreFramePosition = 0;

        // 현재 UI아이템의 번호값 새롭게 생기는곳의 값
        protected int currentItemNo = 0;

        

        private RectTransform _rectTransform;
        protected RectTransform rectTransform
        {
            get
            {
                if (_rectTransform == null) _rectTransform = GetComponent<RectTransform>();
                return _rectTransform;
            }
        }

        private float anchoredPosition
        {
            get
            {
                return m_Direction == Direction.Vertical ? -rectTransform.anchoredPosition.y : rectTransform.anchoredPosition.x;
            }
        }

        private float _itemScale = -1;
        public float itemScale
        {
            get
            {
                if (itemPrototype != null && _itemScale == -1)
                {
                    _itemScale = m_Direction == Direction.Vertical ? itemPrototype.sizeDelta.y : itemPrototype.sizeDelta.x;

                    if(_itemScale < 0)
                    {
                        Debug.LogErrorFormat("에러 아이템 사이즈 이상함 : {0}, {1} ", _itemScale, m_Direction);
                        _itemScale = -1;
                    }
                }
                return _itemScale;
            }
        }

        protected override void Start()
        {
            // 콤포넌트중에 IInfiniteScrollSetup 있는 콤포넌트들 전체적으로 읽어오기
            var controllers = GetComponents<MonoBehaviour>()
                    .Where(item => item is IInfiniteScrollSetup)
                    .Select(item => item as IInfiniteScrollSetup)
                    .ToList();

            // ScrollRect 에서 horizontal 설정하도록 하기
            var scrollRect = GetComponentInParent<ScrollRect>();
            scrollRect.horizontal = m_Direction == Direction.Horizontal;
            scrollRect.vertical = m_Direction == Direction.Vertical;
            scrollRect.content = rectTransform;


            // 복제할 아이템 안보이도록 하기
            itemPrototype.gameObject.SetActive(false);

            for (int i = 0; i < instantateItemCount; i++)
            {
                // create items
                var item = GameObject.Instantiate(itemPrototype) as RectTransform;
                item.SetParent(transform, false);
                item.name = i.ToString();
                item.anchoredPosition = m_Direction == Direction.Vertical ? new Vector2(0, -itemScale * i) : new Vector2(itemScale * i, 0);
                itemList.AddLast(item);

                item.gameObject.SetActive(true);

                // 복제한 오브젝트에 초기화 하기
                foreach (var controller in controllers)
                {
                    controller.OnUpdateItem(i, item.gameObject);
                }
            }

            // 초기화 시켜주기
            foreach (var controller in controllers)
            {
                controller.OnPostSetupItems();
            }
        }

        void Update()
        {
            if (itemList.First == null)
            {
                return;
            }

            if(itemScale <= -1)
            {
                return;
            }

            // itemScale -1 이면 이상하게 꼬여있게 되어있음

            // 앵커 위치가 아이템사이즈보다 커질때 + 부분으로 처리하기위한 값
            // while로 적용한것은 스크롤에서 바로 바뀌게 되었을시를 대비한것
            while (anchoredPosition - diffPreFramePosition < -itemScale * 2)
            {
                diffPreFramePosition -= itemScale;

                var item = itemList.First.Value;
                itemList.RemoveFirst();
                itemList.AddLast(item);

                // 위치지정
                var pos = itemScale * instantateItemCount + itemScale * currentItemNo;
                item.anchoredPosition = (m_Direction == Direction.Vertical) ? new Vector2(0, -pos) : new Vector2(pos, 0);

                // 이벤트 호출
                onUpdateItem.Invoke(currentItemNo + instantateItemCount, item.gameObject);

                currentItemNo++;
            }

            // 앵커 위치가 아이템 사이즈보다 작아질때 - 부분 처리위한값
            while (anchoredPosition - diffPreFramePosition > 0)
            {
                diffPreFramePosition += itemScale;

                var item = itemList.Last.Value;
                itemList.RemoveLast();
                itemList.AddFirst(item);

                currentItemNo--;

                // 위치지정
                var pos = itemScale * currentItemNo;
                item.anchoredPosition = (m_Direction == Direction.Vertical) ? new Vector2(0, -pos) : new Vector2(pos, 0);

                // 이벤트 호출
                onUpdateItem.Invoke(currentItemNo, item.gameObject);
            }
        }
    }
}