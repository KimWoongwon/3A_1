using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.Events;

namespace Du3Project
{

    // https://docs.unity3d.com/kr/current/ScriptReference/Events.UnityEvent.html
    [System.Serializable]
    public class TopLoadingScrollCall : UnityEvent<GameObject>
    {

    }

    public class TopLoadingScroll : MonoBehaviour
    {
        [Header("[속성값]")]
        public Image TopLoadingImage = null;
        public float ItemLoadingSize = 60f;
        public float RotationSpeed = 1f;
        [Tooltip("[원상복귀용 값]")]
        public float LerpSpeed = 0.2f;

        [Header("[콜백용]")]
        public TopLoadingScrollCall CallActionFNs;


        //[Header("[확인값]")]
        //[SerializeField]
        protected ScrollRect m_LinkScrollRect = null;
        //[SerializeField]
        private bool m_ISLoading = false;
        //[SerializeField]
        private float m_ContentY = 0f;


        [Header("[루프값용]")]
        //[SerializeField]
        int m_LoopCount = 0;
        //[SerializeField]
        int m_LoopCountSize = 100;


        Coroutine m_LoadingCoroutine = null;

        public void UIUpdate()
        {
            if (m_LinkScrollRect == null)
                return;

            if (m_ISLoading)
                return;


            m_ContentY = m_LinkScrollRect.content.anchoredPosition.y;

            if (m_ContentY <= -ItemLoadingSize)
            {
                SetStartLoading();
            }
            else if (m_ContentY < 0f)
            {
                float div = 360 / ItemLoadingSize;
                TopLoadingImage.rectTransform.localRotation = Quaternion.Euler(0f, 0f, m_ContentY * div);
            }
        }

        protected void SetStartLoading()
        {
            if (m_LoadingCoroutine != null)
            {
                StopCoroutine(m_LoadingCoroutine);
                m_LoadingCoroutine = null;
            }

            m_LoadingCoroutine = StartCoroutine(RotationLoading());
        }

        IEnumerator RotationLoading()
        {
            m_ISLoading = true;
            TopLoadingImage.rectTransform.localRotation = Quaternion.Euler(0f, 0f, 0f);
            m_LinkScrollRect.enabled = false;

            m_LoopCount = 0;
            while (m_LoopCount <= m_LoopCountSize)
            {
                TopLoadingImage.transform.Rotate(0f, 0f, RotationSpeed);
                m_LoopCount++;
                yield return null;
            }

            m_LinkScrollRect.enabled = true;
            Vector3 targetpos = m_LinkScrollRect.content.localPosition;
            targetpos.y = 0f;
            while (m_LinkScrollRect.content.localPosition.y <= -0.01f)
            {
                Vector3 temppos = Vector3.Lerp(m_LinkScrollRect.content.localPosition, targetpos, LerpSpeed);
                m_LinkScrollRect.content.localPosition = temppos;
                yield return null;
            }

            m_LinkScrollRect.content.localPosition = targetpos;

            LoadingCompleteCallFN();
            yield break;
        }

        public void LoadingCompleteCallFN()
        {
            m_LoadingCoroutine = null;

            m_LoopCount = 0;
            m_ISLoading = false;
            CallActionFNs.Invoke(null);
        }

        private void Awake()
        {
            m_LinkScrollRect = GetComponent<ScrollRect>();

        }

        void Start()
        {

        }

        void Update()
        {
            UIUpdate();

        }
    }
}