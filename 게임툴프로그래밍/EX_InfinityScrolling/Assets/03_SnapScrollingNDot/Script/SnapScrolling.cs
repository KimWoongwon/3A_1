using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;




// 기본 아이디어는 드래그를 떼었을때 Content의 현재 위치를 기준으로
// 해당아이템의 인덱스를 얻어내서 그 인덱스의 위치값을 알아내서
// 강제로 위치로 이동시키는것방식으로 처리하고 있음


// uiextention 이용한 방식
// https://www.youtube.com/watch?v=LnKy3_ymEXs
// https://www.youtube.com/watch?v=KJlIlWHlfMo


// 이곳 참고할것임
// https://www.youtube.com/watch?v=ETqdTIUSf3M
namespace Du3Project
{
    [RequireComponent(typeof(ScrollRect))]
	public class SnapScrolling : MonoBehaviour
        , IBeginDragHandler
        , IEndDragHandler
    {
        // ScrollRect 의 Content 연결하면됨
        public RectTransform panel;
        public Button[] bttn;
        // 가운데 지정용 위치값용 편한것 사용하면됨
        public RectTransform center;
        public int startButton = 1; // 위치지정용값

        public int MaxCount = 5;

        [SerializeField, Header("[값확인용]")]
        private float[] distance;
        //[SerializeField]
        public float[] distReposition;

        [Header("[값확인용2]")]
        [SerializeField]
        private bool draging = false;
        [SerializeField]
        private int bttnDistance;
        [SerializeField]
        private int minButtonNum;
        [SerializeField]
        private int bttnLength;
        [SerializeField]
        private bool messageSend = false;
        [SerializeField]
        private float LerpSpeed = 5f;
        [SerializeField]
        private bool targetNearestButton = true;


        [SerializeField]
        private Vector2 m_VectorPos = new Vector2();

        [SerializeField]
        private Vector3 m_OneOffsetPos = new Vector3();

        [SerializeField]
        private Vector3 m_ViewOffsetPos = new Vector3();

        // 화면에 오른쪽으로 보여주는 갯수
        // 갯수 이상되면 왼쪽으로 이동하기위한 수치
        public int ViewButtonSize = 3;

        void Start()
		{
            GetComponent<ScrollRect>().movementType = ScrollRect.MovementType.Unrestricted;
            // 스크롤을 빨리할시 
            // interial 값때문에 천천히 밀리는것이 느려지도록됨 그것때문에 
            // 밀리는 현상처럼 보이게됨



            bttnLength = bttn.Length;
            distance = new float[bttnLength];
            distReposition = new float[bttnLength];

            bttnDistance = (int)Mathf.Abs(bttn[1].GetComponent<RectTransform>().anchoredPosition.x
                - bttn[0].GetComponent<RectTransform>().anchoredPosition.x);

            panel.anchoredPosition = new Vector2((startButton - 1) * -300f, 0f);



            m_OneOffsetPos = bttn[1].GetComponent<Transform>().position
                - bttn[0].GetComponent<Transform>().position;

            m_ViewOffsetPos = m_OneOffsetPos * ViewButtonSize;
        }

		void Update()
		{
            for (int i = 0; i < bttn.Length; ++i)
            {
                distReposition[i] = center.GetComponent<RectTransform>().position.x - bttn[i].GetComponent<RectTransform>().position.x;
                //distance[i] = Mathf.Abs(distance[i]);
                distance[i] = Mathf.Abs(center.transform.position.x - bttn[i].transform.position.x );




                //// 6개일때의 위치값
                //// 원래 상태로 돌리기위한용도
                //if (distReposition[i] > m_ViewOffsetPos.x)
                //{
                //    float curX = bttn[i].GetComponent<RectTransform>().anchoredPosition.x;
                //    float curY = bttn[i].GetComponent<RectTransform>().anchoredPosition.y;

                //    Vector2 newAnchoredPos = new Vector2(curX + (bttnLength * bttnDistance), curY);
                //    bttn[i].GetComponent<RectTransform>().anchoredPosition = newAnchoredPos;
                //}

                //// 6개일때의 위치값
                //// 원래 상태로 돌리기위한용도
                //if (distReposition[i] < -m_ViewOffsetPos.x)
                //{
                //    float curX = bttn[i].GetComponent<RectTransform>().anchoredPosition.x;
                //    float curY = bttn[i].GetComponent<RectTransform>().anchoredPosition.y;

                //    Vector2 newAnchoredPos = new Vector2(curX - (bttnLength * bttnDistance), curY);
                //    bttn[i].GetComponent<RectTransform>().anchoredPosition = newAnchoredPos;
                //}


            }


            if (targetNearestButton)
            {
                float minDistance = Mathf.Min(distance);

                for (int i = 0; i < bttn.Length; ++i)
                {
                    if (minDistance == distance[i])
                    {
                        minButtonNum = i;
                    }
                }
            }

            if (!draging)
            {
                //LerpToBttn(-bttn[minButtonNum].GetComponent<RectTransform>().anchoredPosition.x);
                LerpToBttn( (float)minButtonNum * -bttnDistance);
            }

            m_VectorPos = panel.anchoredPosition;
        }



        [SerializeField]
        float m_Lerp = 0f;
        void LerpToBttn(float position)
        {
            float lerp = Time.deltaTime* LerpSpeed;
            if (lerp > 1f)
            {
                lerp = 1f;
            }

            m_Lerp = lerp;

            float newX = Mathf.Lerp(panel.anchoredPosition.x, position, m_Lerp);

            if (Mathf.Abs(position - newX) < 3f)
                newX = position;

            if (Mathf.Abs(newX) >= Mathf.Abs(position) - 1f
                && Mathf.Abs(newX) <= Mathf.Abs(position) + 1f
                && !messageSend)
            {
                messageSend = true;
                SendMessageFromButton(minButtonNum);
            }

            Vector2 newPosition = new Vector2(newX, panel.anchoredPosition.y);
            panel.anchoredPosition = newPosition;


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

        public void _On_GoToButton(int buttonIndex)
        {
            // 버턴 눌러서 이동하도록 처리됨
            targetNearestButton = false;
            minButtonNum = buttonIndex - 1;

        }

        public void _On_NextSnap()
        {
            if( minButtonNum + 1 >= MaxCount)
                return;

            _On_GoToButton(minButtonNum + 2);
        }

        public void _On_PrevSnap()
        {
            if (minButtonNum <= 0)
                return;

            _On_GoToButton(minButtonNum);
        }

        public void OnBeginDrag(PointerEventData eventData)
        {
            //throw new System.NotImplementedException();
            StartDrag();
        }
        public void OnEndDrag(PointerEventData eventData)
        {
            //throw new System.NotImplementedException();
            EndDrag();
        }


    }

}