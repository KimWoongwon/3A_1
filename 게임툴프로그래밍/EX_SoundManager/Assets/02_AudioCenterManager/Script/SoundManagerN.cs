using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DG.Tweening;
using System;
using UnityEngine.Events;

namespace Du3Project
{
    [RequireComponent(typeof(AudioSource))]
    public class SoundManagerN : MonoBehaviour
	{

        #region 싱글톤

        static SoundManagerN m_Instance = null;

        public static SoundManagerN GetI
        {
            get
            {
                if (m_Instance == null)
                {
                    m_Instance = GameObject.FindObjectOfType<SoundManagerN>();
                    if (m_Instance == null)
                    {
                        Debug.LogErrorFormat("SoundManagerN 가 씬안에 없습니다. 확인하세요");
                    }
                }

                return m_Instance;
            }
        }
        #endregion


        // 사운드 옵션에 대한 이벤트
        //public static Action<bool, bool> EventUpdateAllSound = delegate { };
        public UnityAction<bool, bool> EventUpdateAllSound = delegate { };

        private E_SSOUND_NAME m_CurrentThemeSound = E_SSOUND_NAME.Max;
        private Dictionary<E_SSOUND_NAME, AudioSource> m_SoundBGMDic = new Dictionary<E_SSOUND_NAME, AudioSource>();

        //private Hashtable m_SoundSFX;
        private AudioSource m_LinkSFXAudioSource = null;
        public UnityEngine.AudioSource LinkSFXAudioSource
        {
            get { return m_LinkSFXAudioSource; }
        }

        private bool m_ISBGMSound = true;
        public bool ISBGMSound
        {
            get { return m_ISBGMSound; }
            set
            {
                m_ISBGMSound = value;
                CheckAllSound();
            }
        }

        private bool m_ISSFXSound = true;
        public bool ISSFXSound
        {
            get { return m_ISSFXSound; }
            set
            {
                m_ISSFXSound = value;
                CheckAllSound();
            }
        }



        public float VolumeDurationSec = 1f;
        public float MinSameSFXSoundDelaySec = 0.1f;




        //public UnityAction<E_SSOUND_NAME> SoundPlayEvent;
        /// <summary>
        /// 사운드 매니저에 직접 호출하는 방식
        /// </summary>
        /// <param name="p_click"></param>
        [EnumAction(typeof(E_SUISoundType))]
        public void _On_SetUIClickSound(int p_click)
        {
            ShotSoundSFX((E_SUISoundType)p_click);

        }

        [EnumAction(typeof(E_SSOUND_NAME))]
        public void _On_SetClickPlaySound(int p_click)
        {
            SetPlaySound((E_SSOUND_NAME)p_click);

        }

        [EnumAction(typeof(E_SSOUND_NAME))]
        public void _On_SetClickStopSound(int p_click)
        {
            SetStopSound((E_SSOUND_NAME)p_click);
        }

        [EnumAction(typeof(E_SSOUND_NAME))]
        public void _On_SetOnlyBGMPlaySound(int p_click)
        {
            SetPlaySoundBGM((E_SSOUND_NAME)p_click);
        }



        //public void SoundPlaySFX(AudioClip p_clip)
        //{
        //    if(m_ISSFXSound == false)
        //        return;


        //}

        //public void PlayBGM(AudioClip p_clip)
        //{
        //    if (m_ISBGMSound == false)
        //        return;


        //}


        bool m_ISInit = false;
        void InitSettingSoundManager()
        {
            if (m_ISInit)
                return;


            m_BackUpStateBG = m_ISBGMSound;
            m_BackUpStateSFX = m_ISSFXSound;

            foreach (var item in SoundLinkList)
            {
                m_AudioClipURLDic.Add(item.SoundType, item.SoundClip);
            }

            // 실시간 등록
            foreach (var item in m_UIClickDic)
            {
                GetRuntimeAudioClip(item.Key);
            }

            m_LinkSFXAudioSource = GetComponent<AudioSource>();


            // jar 플러그인에서 사용할것
            Plugin_AudioCenter.GetI.InitDefaultSoundLoader(m_AudioClipURLDic);

            m_ISInit = true;
        }


        void CheckAllSound()
        {
            if (EventUpdateAllSound != null)
                EventUpdateAllSound.Invoke(m_ISBGMSound, m_ISSFXSound);

            SetActiveAllBGMSound(m_ISBGMSound);
            SetActiveAllSFXSound(m_ISSFXSound);
        }


        public class SSoundLinkStringName
        {
            public string ClipURL = "";
            public string ShotURL = "";
        }
        public static Dictionary<E_SSOUND_NAME, float> m_AudioClipStartTime = new Dictionary<E_SSOUND_NAME, float>((int)E_SSOUND_NAME.Max);
        //public static Dictionary<E_SSOUND_NAME, SSoundLinkStringName> m_AudioClipURLDic = 
        //                    new Dictionary<E_SSOUND_NAME, SSoundLinkStringName>()
        //{
        //    // BGM 사운드용
        //    { E_SSOUND_NAME.MainUIBGM0, new SSoundLinkStringName(){ClipURL = "Sound/BGM/LobbyMusic", ShotURL=""} }
        //    , { E_SSOUND_NAME.MainUIBGM1, new SSoundLinkStringName(){ClipURL = "Sound/BGM/LobbyMusic1", ShotURL=""} }
        //    , { E_SSOUND_NAME.InGameBGM0, new SSoundLinkStringName(){ClipURL = "Sound/BGM/InGame_BGM0", ShotURL=""} }
        //    , { E_SSOUND_NAME.InGameBGM1, new SSoundLinkStringName(){ClipURL = "Sound/BGM/InGame_BGM1", ShotURL=""} }
        //    , { E_SSOUND_NAME.InGame_MissionSuccess, new SSoundLinkStringName(){ClipURL = "Sound/BGM/InGame_MissionSuccess", ShotURL=""} }
        //    , { E_SSOUND_NAME.InGame_MissionFail, new SSoundLinkStringName(){ClipURL = "Sound/BGM/InGame_BGM0", ShotURL=""} }

        //};

        [System.Serializable]
        public class SSoundLinkCls
        {
            public E_SSOUND_NAME SoundType = E_SSOUND_NAME.Max;
            public AudioClip SoundClip = null;
        }

        [Header("[사운드 리스트용들]")]
        public List<SSoundLinkCls> SoundLinkList = new List<SSoundLinkCls>();
        protected Dictionary<E_SSOUND_NAME, AudioClip> m_AudioClipURLDic = new Dictionary<E_SSOUND_NAME, AudioClip>();


        // 폴더에 있는것들 전부 자동업데이트 하는 방법도 있음
        private Dictionary<E_SSOUND_NAME, string> m_UIClickDic = new Dictionary<E_SSOUND_NAME, string>()
        {
            {E_SSOUND_NAME.Click0, "01_SoundResource/ShotSound/Click00" }
            , {E_SSOUND_NAME.Click01, "01_SoundResource/ShotSound/Click01" }
            , {E_SSOUND_NAME.Click02, "01_SoundResource/ShotSound/Click02" }
            , {E_SSOUND_NAME.Click03, "01_SoundResource/ShotSound/Click03" }
            , {E_SSOUND_NAME.Click04, "01_SoundResource/ShotSound/Click04" }
            , {E_SSOUND_NAME.Click05_Beep, "01_SoundResource/ShotSound/Click05_Beep" }
            , {E_SSOUND_NAME.CoinUp, "01_SoundResource/ShotSound/CoinUp" }
        };

        //public AudioClip GetRuntimeAudioClip(E_SSOUND_NAME p_soundtype)
        //{
        //    if( !m_AudioClipURLDic.ContainsKey(p_soundtype) )
        //    {
        //        return null;
        //    }

        //    string name = m_AudioClipURLDic[p_soundtype].ClipURL;
        //    return Resources.Load<AudioClip>(name);
        //}




        public AudioClip GetDictAudioClip(string p_soundtype)
        {
            foreach (var item in m_AudioClipURLDic)
            {
                if(item.Key.ToString() == p_soundtype )
                {
                    return item.Value;
                }
            }

            return null;
        }

        public AudioClip GetDictAudioClip(E_SSOUND_NAME p_soundtype)
        {
            if (!m_AudioClipURLDic.ContainsKey(p_soundtype))
            {
                return null;
            }

            return m_AudioClipURLDic[p_soundtype];
        }

        public AudioClip GetRuntimeAudioClip(E_SSOUND_NAME p_soundtype)
        {
            if (m_AudioClipURLDic.ContainsKey(p_soundtype))
            {
                return m_AudioClipURLDic[p_soundtype];
            }

            string name = m_UIClickDic[p_soundtype];
            AudioClip audioClip = Resources.Load<AudioClip>(name);

            if (audioClip == null)
            {
                Debug.LogErrorFormat("Audio Clip NullData : {0}", p_soundtype);
                return null;
            }


            m_AudioClipURLDic.Add(p_soundtype, audioClip);

            return audioClip;
        }

        public void SetStopSound(E_SSOUND_NAME p_soundtype)
        {
            // 강제로 바로 사운드 죽임
            AudioSource audiosource = null;
            if (m_SoundBGMDic.ContainsKey(p_soundtype))
            {
                audiosource = m_SoundBGMDic[p_soundtype];

                audiosource.DOFade(1f, VolumeDurationSec)
                    .OnComplete(() => {
                        audiosource.volume = 0f;
                        audiosource.mute = true;
                        audiosource.Stop();
                    });
            }
        }
        /// <summary>
        /// 여러개의 BGM들을 p_soundtype별로 하나씩 중복으로 플레이 하기위한 방식
        /// p_soundtype 같은 타입이면 플레이가 이어서 되도록 하기
        /// </summary>
        /// <param name="p_soundtype"></param>
        /// <param name="p_isdirect"></param>
        public void SetPlaySound(E_SSOUND_NAME p_soundtype
                    , bool p_isloop = false
                    , bool p_isrestart = false
                    )
        {
            if (!m_ISBGMSound)
                return;


            AudioSource audiosource = null;
            if (!m_SoundBGMDic.ContainsKey(p_soundtype))
            {
                GameObject newobj = new GameObject(p_soundtype.ToString(), typeof(AudioSource));
                audiosource = newobj.GetComponent<AudioSource>();
                newobj.transform.parent = transform;
                m_SoundBGMDic.Add(p_soundtype, audiosource);

                AudioClip clipaudio = GetDictAudioClip(p_soundtype);

                if (clipaudio == null)
                    Debug.LogFormat("AudioClip Null : {0}", p_soundtype);

                audiosource.clip = clipaudio;
                audiosource.spatialBlend = 0f;
            }
            else
            {
                audiosource = m_SoundBGMDic[p_soundtype];
            }


            if (audiosource != null)
            {
                audiosource.mute = false;
                audiosource.loop = p_isloop;
                if (!audiosource.isPlaying || p_isrestart)
                {
                    audiosource.Play();
                    audiosource.volume = 0f;
                    audiosource.DOFade(1f, VolumeDurationSec)
                        .OnComplete(() => {
                            if (!audiosource.isPlaying)
                            {
                                // 딜레이 방식으로 멈추게 하다가 강제로 실행 하다보면 
                                // 멈추는것이 complete가 되면서 stop 가 되는 현상이 있음 그때 강제로 play하기위한 방식임
                                Debug.LogFormat("딜레이 플레이중 사운드 플레이안됨 강제 플레이 : ");
                                audiosource.volume = 1f;
                                audiosource.Play();
                            }
                        }
                        );
                }
            }
            else
            {
                m_SoundBGMDic.Remove(p_soundtype);
            }

        }

        // 강제 플레이 멈추도록 하기
        public void SetStopCurrentBGM()
        {
            if (m_CurrentThemeSound != E_SSOUND_NAME.Max
                && m_SoundBGMDic[m_CurrentThemeSound] != null)
            {
                // 기존 bgm사운드
                AudioSource audiosource = m_SoundBGMDic[m_CurrentThemeSound];
                if (audiosource != null)
                {
                    audiosource.DOFade(0f, VolumeDurationSec)
                        .OnComplete(() => {
                            audiosource.Stop();
                        }
                        );
                }
            }
        }


        /// <summary>
        /// BGM 사운드를 오직 하나만 실행하기 위한 방식
        /// 중복으로 bgm이 플레이 되지 않도록 하기
        /// </summary>
        /// <param name="p_soundtype"></param>
        /// <param name="p_isdirect"></param>
        /// <returns></returns>
        public GameObject SetPlaySoundBGM(E_SSOUND_NAME p_soundtype
                    , bool p_isdirect = false)
        {
            if (!m_ISBGMSound)
                return null;

            GameObject outobj = null;

            if (m_CurrentThemeSound == p_soundtype
                && !p_isdirect)
            {
                return null;
            }

            AudioSource audiosource = null;
            if (!m_SoundBGMDic.ContainsKey(p_soundtype))
            {
                GameObject newobj = new GameObject(p_soundtype.ToString(), typeof(AudioSource));
                audiosource = newobj.GetComponent<AudioSource>();
                newobj.transform.parent = transform;
                m_SoundBGMDic.Add(p_soundtype, audiosource);

                AudioClip clipaudio = GetDictAudioClip(p_soundtype);

                if (clipaudio == null)
                    Debug.LogFormat("AudioClip Null : {0}", p_soundtype);

                audiosource.clip = clipaudio;
                audiosource.loop = true;
                audiosource.spatialBlend = 0f;

            }
            else
            {
                audiosource = m_SoundBGMDic[p_soundtype];

            }

            if (audiosource != null)
            {
                audiosource.Play();
                audiosource.volume = 0f;
                audiosource.DOFade(1f, VolumeDurationSec)
                    .OnComplete(() => {
                        if (!audiosource.isPlaying)
                        {
                            Debug.LogFormat("딜레이 플레이중 사운드 플레이안됨 강제 플레이 : ");
                            audiosource.Play();
                        }
                    }
                    );
            }
            else
            {
                m_SoundBGMDic.Remove(p_soundtype);
            }

            if (m_CurrentThemeSound != E_SSOUND_NAME.Max
                && m_SoundBGMDic[m_CurrentThemeSound] != null)
            {
                // 기존 bgm사운드
                audiosource = m_SoundBGMDic[m_CurrentThemeSound];
                if (audiosource != null)
                {
                    audiosource.DOFade(0f, VolumeDurationSec)
                        .OnComplete(() => {
                            audiosource.Stop();
                        }
                        );
                }
            }

            // 딜레이 사운드 없어지기전에 이미 받아서 처리함
            m_CurrentThemeSound = p_soundtype;
            return null;
        }



        public void ShotSoundSFX(E_SUISoundType p_soundtype)
        {
            ShotSoundSFX((E_SSOUND_NAME)p_soundtype);
        }

        /// <summary>
        /// 이때는 무조건 shot등은 무조건 사운드가 플레이가 된다
        /// </summary>
        /// <param name="p_soundtype"></param>
        public void ShotSoundSFX(E_SSOUND_NAME p_soundtype)
        {
            if (!m_ISSFXSound)
                return;

            if (m_LinkSFXAudioSource == null)
                return;


            // 빠르게 충돌시 중복 막기 위한것
            if (!m_AudioClipStartTime.ContainsKey(p_soundtype))
            {
                m_AudioClipStartTime.Add(p_soundtype, 0f);
            }

            float lastsec = m_AudioClipStartTime[p_soundtype] + MinSameSFXSoundDelaySec;
            float currsec = Time.time;
            if (lastsec > currsec)
            {
                Debug.LogFormat("너무 빠르게 중복됨 : {0}", p_soundtype);
                return;
            }

            //AudioClip clipaudio = GetDictAudioClip(p_soundtype);
            //m_LinkSFXAudioSource.PlayOneShot(clipaudio);


            // shotsound
            Plugin_AudioCenter.playSound( p_soundtype.ToString() );

        }

        void SetActiveAllBGMSound(bool p_isflag)
        {
            foreach (var item in m_SoundBGMDic)
            {
                item.Value.mute = !p_isflag;
            }
        }

        void SetActiveAllSFXSound(bool p_isflag)
        {
            //foreach (AudioSource item in m_SoundSFX.Values )
            //{
            //    item.mute = p_isflag;
            //}

            // shot등은 무조건 소리가나오고 있기때문에 mute로 강제로 음소거를 하도록 한다
            m_LinkSFXAudioSource.mute = !p_isflag;
        }


        private bool m_BackUpStateBG = true;
        private bool m_BackUpStateSFX = true;
        public void TurnOffSoundTemporary()
        {
            m_BackUpStateBG = ISBGMSound;
            m_BackUpStateSFX = ISSFXSound;

            ISBGMSound = false;
            ISSFXSound = false;
        }

        public void RecoverSoundState()
        {
            ISBGMSound = m_BackUpStateBG;
            ISSFXSound = m_BackUpStateSFX;

        }

        // 모바일 일시정지에 사용하기 위한것
        void OnApplicationPause(bool pause)
        {
            Debug.Log("AllSoundManager Pause : " + pause);
            if (pause)
            {
                TurnOffSoundTemporary();
            }
            else
            {
                RecoverSoundState();
            }
        }


        private void Awake()
        {
            InitSettingSoundManager();

        }

        void Start()
        {

        }

        void Update()
        {

        }


    }
}