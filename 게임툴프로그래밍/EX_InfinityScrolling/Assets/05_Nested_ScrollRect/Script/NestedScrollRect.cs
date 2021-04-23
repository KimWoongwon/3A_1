using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;



// 참고용 소스
// https://gist.github.com/Josef212/8d296acdd4f457e39e0c13fbc9ec007e
// 기본적으로  m_parentScrollRect Inspector 에서 debug모드를 이용해서 강제로 상단 ScrollRect를 연결 하면됨
namespace Du3Project
{
    public class NestedScrollRect : ScrollRect
    {
        // 이벤트를 가로채서 드래그 하도록 처리함 
        public override void OnInitializePotentialDrag(PointerEventData eventData)
        {
            for (int i = 0; i < m_parentInitializePotentialDragHandlers.Length; ++i)
            {
                m_parentInitializePotentialDragHandlers[i].OnInitializePotentialDrag(eventData);
            }
            base.OnInitializePotentialDrag(eventData);
        }

        public override void OnBeginDrag(PointerEventData eventData)
        {
            m_routeToParent = (!horizontal && Mathf.Abs(eventData.delta.x) > Mathf.Abs(eventData.delta.y)) || (!vertical && Mathf.Abs(eventData.delta.x) < Mathf.Abs(eventData.delta.y));

            if (m_routeToParent)
            {
                for (int i = 0; i < m_parentBeginDragHandlers.Length; ++i)
                {
                    m_parentBeginDragHandlers[i].OnBeginDrag(eventData);
                }
            }
            else
            {
                base.OnBeginDrag(eventData);
            }
        }

        public override void OnDrag(PointerEventData eventData)
        {
            if (m_routeToParent)
            {
                for (int i = 0; i < m_parentDragHandlers.Length; ++i)
                {
                    m_parentDragHandlers[i].OnDrag(eventData);
                }
            }
            else
            {
                base.OnDrag(eventData);
            }
        }

        public override void OnEndDrag(PointerEventData eventData)
        {
            if (m_routeToParent)
            {
                for (int i = 0; i < m_parentEndDragHandlers.Length; ++i)
                {
                    m_parentEndDragHandlers[i].OnEndDrag(eventData);
                }
            }
            else
            {
                base.OnEndDrag(eventData);
            }

            m_routeToParent = false;
        }

        protected override void Awake()
        {
            base.Awake();

            if (m_parentScrollRect == null)
            {
                throw new System.NullReferenceException("Parent Scroll Rect is null");
            }

            // draghandle핸들 값들 강제로 받아오기 위한 변수들
            m_parentInitializePotentialDragHandlers = m_parentScrollRect.GetComponents<IInitializePotentialDragHandler>();
            m_parentBeginDragHandlers = m_parentScrollRect.GetComponents<IBeginDragHandler>();
            m_parentDragHandlers = m_parentScrollRect.GetComponents<IDragHandler>();
            m_parentEndDragHandlers = m_parentScrollRect.GetComponents<IEndDragHandler>();
        }

        [SerializeField] private ScrollRect m_parentScrollRect = null;

        private bool m_routeToParent = false;

        private IInitializePotentialDragHandler[] m_parentInitializePotentialDragHandlers = null;
        private IBeginDragHandler[] m_parentBeginDragHandlers = null;
        private IDragHandler[] m_parentDragHandlers = null;
        private IEndDragHandler[] m_parentEndDragHandlers = null;
    }
}


