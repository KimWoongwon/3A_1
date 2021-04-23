using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using static Du3Project.InfiniteScroll;

namespace Du3Project
{
    [RequireComponent(typeof(ScrollRect))]
    public class UIInfinitySnapControl : MonoBehaviour
        , IBeginDragHandler
        , IEndDragHandler
    {
        public UIItemSnapControlLimite LinkSnapControllLimite = null;

        #region 참고용 자료들임 SerializeField

        
        [Header("[참고용자료들]")]
        //[SerializeField]
        private RectTransform Content;
        //[SerializeField]
        ScrollRect m_ParentScroll = null;
        //[SerializeField]
        private int MaxItemCount = 5;


        //[SerializeField]
        private bool messageSend = false;
        //[SerializeField]
        private float LerpSpeed = 5f;
        //[SerializeField]
        private bool targetNearestButton = true;
        //[SerializeField]
        private bool draging = false;

        //[SerializeField]
        private float m_ItemScale = 0f;
        //[SerializeField]
        private float m_DivItemScale = 0f;
        //[SerializeField]
        private Direction m_Direction;
        //[SerializeField]
        private int minButtonNum;
        //[SerializeField]
        private Vector2 m_AnchorPos = new Vector2();


        #endregion


        //internal void SetSnapInitSetting( int p_maxcount)
        //{
        //    MaxCount = p_maxcount;

        //}


        private void Awake()
        {
            m_ParentScroll = GetComponentInParent<ScrollRect>();
            if(m_Direction == InfiniteScroll.Direction.Vertical)
            {
                m_ParentScroll.vertical = true;
                m_ParentScroll.horizontal = false;
            }
            else
            {
                m_ParentScroll.vertical = false;
                m_ParentScroll.horizontal = true;
            }
            
            Content = m_ParentScroll.content;


            if(LinkSnapControllLimite == null)
                LinkSnapControllLimite = GetComponentInChildren<UIItemSnapControlLimite>();

            // start 하기 전에 호출해야지됨
            LinkSnapControllLimite.GetComponent<InfiniteScroll>().DirectionVal = m_Direction;
            MaxItemCount = LinkSnapControllLimite.ItemCount;

            GetComponent<ScrollRect>().movementType = ScrollRect.MovementType.Unrestricted;


            InfiniteScroll infinityscroll = LinkSnapControllLimite.GetComponent<InfiniteScroll>();
            m_ItemScale = infinityscroll.itemScale;
            m_Direction = infinityscroll.DirectionVal;
            m_DivItemScale = m_ItemScale * 0.5f;
        }


        void Start()
		{
			
		}


        [SerializeField]
        protected int m_TempMinDistance = 0;
		void Update()
		{

            // 근처 위치값 알기위한 함수
            if (targetNearestButton)
            {
                Vector2 currpos = Content.anchoredPosition;

                if( m_Direction == InfiniteScroll.Direction.Vertical)
                {
                    // 올라가면 +가 되기때문임
                    m_TempMinDistance = (int)((currpos.y + m_DivItemScale) / m_ItemScale);
                    minButtonNum = m_TempMinDistance;
                }
                else
                {
                    // 오른쪽이동이면 x쪽이 -가 됨 
                    m_TempMinDistance = (int)((-currpos.x + m_DivItemScale) / m_ItemScale);
                    minButtonNum = m_TempMinDistance;
                }


                if(minButtonNum < 0)
                {
                    minButtonNum = 0;
                }

                //float minDistance = Mathf.Min(distance);

                //for (int i = 0; i < bttn.Length; ++i)
                //{
                //    if (minDistance == distance[i])
                //    {
                //        minButtonNum = i;
                //    }
                //}
            }

            if (!draging)
            {
                //LerpToBttn(-bttn[minButtonNum].GetComponent<RectTransform>().anchoredPosition.x);
                LerpToBttn((float)minButtonNum * -m_ItemScale);
            }

            m_AnchorPos = Content.anchoredPosition;
        }

        [SerializeField]
        float m_Lerp = 0f;
        void LerpToBttn(float position)
        {
            float lerp = Time.deltaTime * LerpSpeed;
            if (lerp > 1f)
            {
                lerp = 1f;
            }

            m_Lerp = lerp;


            Vector2 newPosition = Content.anchoredPosition;// new Vector2(newX, Content.anchoredPosition.y);
            if (m_Direction == InfiniteScroll.Direction.Horizontal)
            {
                float newX = Mathf.Lerp(Content.anchoredPosition.x, position, m_Lerp);

                if (Mathf.Abs(position - newX) < 3f)
                    newX = position;

                if (Mathf.Abs(newX) >= Mathf.Abs(position) - 1f
                    && Mathf.Abs(newX) <= Mathf.Abs(position) + 1f
                    && !messageSend)
                {
                    messageSend = true;
                    SendMessageFromButton(minButtonNum);
                }
                newPosition.x = newX;
            }
            else
            {
                position = -1f * position;
                float newY = Mathf.Lerp(Content.anchoredPosition.y, position, m_Lerp);

                if (Mathf.Abs(position - newY) < 3f)
                    newY = position;

                if (Mathf.Abs(newY) >= Mathf.Abs(position) - 1f
                    && Mathf.Abs(newY) <= Mathf.Abs(position) + 1f
                    && !messageSend)
                {
                    messageSend = true;
                    SendMessageFromButton(minButtonNum);
                }
                newPosition.y = newY;
            }


            Content.anchoredPosition = newPosition;

        }

        void SendMessageFromButton(int bttnIndex)
        {
            if (bttnIndex - 1 == 3)
                Debug.LogFormat("Message Send");

        }


        public void StartDrag()
        {
            messageSend = false;
            //LerpSpeed = 5f;
            draging = true;

            targetNearestButton = true;
        }

        public void EndDrag()
        {
            draging = false;
        }

        public void OnBeginDrag(PointerEventData eventData)
        {
            StartDrag();
        }

        public void OnEndDrag(PointerEventData eventData)
        {
            EndDrag();
        }

        public void _On_GoToButton(int buttonIndex)
        {
            if( buttonIndex <= 0
                || buttonIndex >= MaxItemCount)
            {
                return;
            }

            // 버턴 눌러서 이동하도록 처리됨
            targetNearestButton = false;
            minButtonNum = buttonIndex;

        }
    }
}