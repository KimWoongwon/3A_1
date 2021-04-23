using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DG.Tweening;


public enum E_TestType
{
    TestA = 0,
    TestB,
    TestC,
    Max
}

// 샘플용임 게임별 소스 따로 만들어 둬야지됨
public class EX_SoundObjectManager : Singleton_Mono<EX_SoundObjectManager>
{

    [Header("{BGM사운드]")]
    public AudioSource[] BGMSoundArr = null;

    private AudioSource m_PlayAudioSource = null;

    // AudioClip 를 사용하면 파일에 같이 실려서 저장이됨
    //     public void _On_ShotSoundPlayClip(AudioClip p_audioclip)
    //     {
    //         if (PlayerDataManager.GetI.OptionDatas.ISEffectSound)
    //         {
    //             EX_AudioCenter.PlaySound(p_audioclip);
    //         }
    // 
    //     }


    [EX_AudioClipAttribute]
    public void _On_TestShotSondPlayClip(string p_audiostr)
    {

    }

    [EnumActionAttribute(typeof(E_TestType))]
    public void _On_TestEnumType(int p_enum)
    {

    }

    public void _On_ShotSoundPlayEnum(int p_enum)
    {

    }


    protected bool m_ISEffectSound = true;
    protected bool m_ISBGMSound = true;
    public bool ISEffectSound
    {
        get { return m_ISEffectSound; }
        private set { m_ISEffectSound = value; }
    }
    public bool ISBGMSound
    {
        get { return m_ISBGMSound; }
        private set { m_ISBGMSound = value; }
    }

    public void _On_ShotSoundPlay(string p_audiostr)
    {
        if (m_ISEffectSound)
        {
            EX_AudioCenter.PlaySound(p_audiostr);
        }
    }

    public void SetOKClickEffectSound()
    {
        if (m_ISEffectSound)
        {
            EX_AudioCenter.PlaySound("menu_select_2");
        }
    }



    public void _On_ShotOKClick()
    {
        SetOKClickEffectSound();
    }



    public float BGMVolume = 0.2f;
    public void InitBGMSoundPlay()
    {
        int count = BGMSoundArr.Length;

        if (count <= 0)
            return;

        for (int i = 0; i < count; ++i)
        {
            BGMSoundArr[i].Stop();
        }

    }

    public void SetOptionBGMSound()
    {
        SetBGMSoundUse(m_ISBGMSound);

    }

    public void SetBGMSoundUse(bool p_flag)
    {
        if (p_flag)
        {
//             if (m_PlayAudioSource != null)
//             {
//                 m_PlayAudioSource.Stop();
//                 m_PlayAudioSource = null;
//             }
// 
// 
//             int randomindex = Random.Range(0, BGMSoundArr.Length);
//             BGMSoundArr[randomindex].volume = BGMVolume;
//             m_PlayAudioSource = BGMSoundArr[randomindex];
//             if ( !BGMSoundArr[randomindex].isPlaying )
//             {
//                 BGMSoundArr[randomindex].Play();
//             }


            PlayRandomBGMSound();

        }
        else
        {
            int count = BGMSoundArr.Length;
            for (int i = 0; i < count; ++i)
            {
                BGMSoundArr[i].Stop();
            }
        }

    }




    [Header("[사운드페이드방식]")]
    public float FadeDurationSec = 0.5f;
    AudioSource m_CurrentAudioSource = null;
    Tween m_BGMSoundTween = null;
    Tween m_BGMFadeoutTween = null;
    Tween m_BGMFadeinTween = null;
    Tween m_BGMDelayTween = null;
    protected void StopBGMFadeInTween()
    {
        if (m_BGMFadeinTween != null)
        {
            m_BGMFadeinTween.Kill();
            m_BGMFadeinTween = null;
        }
    }

    protected void StopBGMFadeOutTween()
    {
        if (m_BGMFadeoutTween != null)
        {
            m_BGMFadeoutTween.Kill();
            m_BGMFadeoutTween = null;
        }
    }

    protected void PlayRandomBGMSound()
    {
        StopBGMFadeInTween();
        //StopBGMFadeOutTween();


        if (!m_ISBGMSound)
        {
            return;
        }




        // 같은것이 플레이 될수 있음
        int randomindex = Random.Range(0, BGMSoundArr.Length);
        m_CurrentAudioSource = BGMSoundArr[randomindex];


        //         if (m_CurrentAudioSource != m_PrevAudioSource)
        //         {
        //             m_CurrentAudioSource.volume = 0f;
        //             if (!m_CurrentAudioSource.isPlaying)
        //                 m_CurrentAudioSource.Play();
        //         }



        m_CurrentAudioSource.Stop();
        m_CurrentAudioSource.volume = 0f;
        m_CurrentAudioSource.Play();



        m_BGMFadeinTween = m_CurrentAudioSource.DOFade(BGMVolume, FadeDurationSec);
        m_BGMDelayTween = DOVirtual.DelayedCall(m_CurrentAudioSource.clip.length - FadeDurationSec, PlayTweenDelaySoundComplete);


        Debug.LogFormat("DelayVal : {0}, {1}, {2}"
            , m_CurrentAudioSource.name
            , m_CurrentAudioSource.clip.name
            , m_CurrentAudioSource.clip.length - FadeDurationSec);
    }

    protected AudioSource m_PrevAudioSource = null;
    protected void PlayTweenDelaySoundComplete()
    {

        Debug.LogFormat("delay Com : ");

        m_BGMDelayTween = null;

        m_PrevAudioSource = m_CurrentAudioSource;
        StopBGMFadeOutTween();
        m_BGMFadeoutTween = m_PrevAudioSource.DOFade(0f, FadeDurationSec)
            .OnComplete(PlayEndSound);

        PlayRandomBGMSound();
    }

    protected void PlayEndSound()
    {
        m_BGMFadeoutTween = null;

        if (m_PrevAudioSource == null)
            return;

        if (m_PrevAudioSource == m_CurrentAudioSource)
        {
            m_PrevAudioSource = null;
            return;
        }


        //m_PrevAudioSource.mute = true;
        m_PrevAudioSource.Stop();
        m_PrevAudioSource = null;
    }






    void Start()
    {
        InitBGMSoundPlay();
    }



    /*
    //protected Dictionary<E_BGMType, AudioClip> m_BGMDic = new Dictionary<E_BGMType, AudioClip>();
    [SerializeField]
    protected List<AudioClip> m_BGMList = new List<AudioClip>((int)E_BGMType.Max);
    public List<AudioClip> BGMList
    {
        get { return m_BGMList; }
    }

    protected void InitSettingBGMList()
    {
        int count = m_BGMList.Count;

        if ((int)E_BGMType.Max != count)
        {
            Debug.LogErrorFormat("BGMList Enum OverCount : {0}", count);
            m_BGMList.Clear();
            count = (int)E_BGMType.Max;
            for (int i = 0; i < count; ++i)
            {
                m_BGMList.Add(null);
            }
        }

    }

     * */

}
