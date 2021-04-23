using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System;
using DG.Tweening;



public enum E_BGMType
{
    MainMenuBGM = 0,
    InGameBGM,
    InGameBonusBGM,
    InGameScreamBGM,
    InGameResultBGM,

    Max
}

public enum E_ButtonSoundType
{
    None = 0,
    OKBTN,
    Cancel,
    Back,
    Ignore,
    Retry,
    Next,
    MenuOpen,
    DefaultSkill,
    BallSkill,
    Click0,
    Click1,
    Click2,
    Click3,

    Max
}


public enum SOUND_NAME 
{
	Shoot,
	GoalKeeper_Catch,
	Whistle,
	Crowd_Goal,
	Crowd_Out,
	Button,
	Ball_Hit_Bar,
	Ball_Hit_Net,
	Ball_Hit_Player_Wall,
	Ball_Hit_Goal,
	Ball_Hit_Goal_Extra,
	BG_Crowd,
	BG_Menu,
	BG_Menu1,



    // ShotSound
    Keeper_Punch,
    
    InGame_StartCount, // 게임시작시 3,2,1 카운터 소리
    InGame_StartWhistle, // 게임시작 휘슬
    InGame_EndWhistle, // 게임종료 휘슬
    InGame_GoalPostHit, // 골포스터히트
    InGame_BallShootWind0, // 공날아가는 소리
    InGame_BallShootWind1, // 공날아가는 소리
    InGame_NetHit0, // 네트부딪히는 소리
    InGame_NetHit1, // 네트부딪히는 소리
    InGame_Sighs, // 골실패시 아쉬운소리
    InGame_PantingMale, // 힘들어하는 소리
    InGame_BuBuJella, // 부부젤라
    InGame_Scrim00, // 게임중소리
    InGame_Scrim01, // 게임중소리
    InGame_Scrim02, // 게임중소리
    InGame_Scrim03, // 게임중소리
    InGame_Scrim04, // 게임중소리
    InGame_FireCracker, // 골넣었을때 폭죽소리
    InGame_GroundHit0, // 땅에충돌할때소리
    InGame_GroundHit1, // 땅에충돌할때소리
    InGame_GroundHit2, // 땅에충돌할때소리
    InGame_PenceHit0, // 펜스충돌
    InGame_PenceHit1, // 펜스충돌
    InGame_PenceHit2, // 펜스충돌
    InGame_MultiOtherAddScore, // 멀티 스코어넣었을때

    // InGame ShotSound


    // BGM
    MainUIBGM0, // 메인ui에서적용
    MainUIBGM1,
    InGameBGM0, // 게임실행시 bgm
    InGameBGM1,
    InGame_MissionSuccess, // 미션 성공
    InGame_MissionFail, // 미션 실패

	None
}

public struct DicSoundURL 
{
    public string ManagerSoundURL; // soundmanager 에서 사용할것
    public string AudioCenterName; // audiocentername 에서 사용할것
}
public class SoundManager : MonoBehaviour {


	public static SoundManager share;
    public static Action<bool, bool> EventRecheckAllSound = delegate {};

	private Hashtable soundBG;
	private Hashtable soundSFX;

	private SOUND_NAME currentThemeSound;
	
	private bool isSFXOn 	= true;

	public bool IsSFXOn {
		get {
			return isSFXOn;
		}
		set {
			isSFXOn = value;
			recheckAllSound();
		}
	}

	private bool isBGMusicOn = true;

	public bool IsBGMusicOn {
		get {
			return isBGMusicOn;
		}
		set {
			isBGMusicOn = value;
			recheckAllSound();
		}
	}

	private bool _backUpStateSFX;
	private bool _backUpStateBG;

	public void turnOffSoundTemporary() {
		_backUpStateSFX = IsSFXOn;
		_backUpStateBG = IsBGMusicOn;

		IsSFXOn = false;
		IsBGMusicOn = false;
	}

	public void recoverSoundState() {
		IsSFXOn = _backUpStateSFX;
		IsBGMusicOn = _backUpStateBG;
	}

	public ArrayList sounds;
	

	void Awake() 
    {
		share = this;
		sounds = new ArrayList();
		soundBG = new Hashtable();
		soundSFX = new Hashtable();
		currentThemeSound = SOUND_NAME.None;

		_backUpStateBG = true;
		_backUpStateSFX = true;



        m_AudioClipStartTime.Clear();
        int count = (int)SOUND_NAME.None;
        for (int i=0; i< count; ++i)
        {
            m_AudioClipStartTime.Add((SOUND_NAME)i, 0f);
        }
	}

	void Start() 
    {
//         if (PlayerDataManager.GetI != null)
//         {
//             isBGMusicOn = PlayerDataManager.GetI.PPlayerOptionData.ISBGM;
//             isSFXOn = PlayerDataManager.GetI.PPlayerOptionData.ISEffectSound;
//         }
// 
//         Shoot.EventOnCollisionEnter += BallEvent_OnCollisionEnter;
        


		_backUpStateBG = IsBGMusicOn;
		_backUpStateSFX = IsSFXOn;
	}

	private void clearAll() 
    {
		soundBG.Clear();
		soundSFX.Clear();
	}
	
	private void recheckAllSound()
	{
	    EventRecheckAllSound(isBGMusicOn, isSFXOn);

		if(!isBGMusicOn) {
			muteAllBGMusic();
		}
		else {
			unMuteAllCurrentBGMusic();
		}
		
		if(!isSFXOn)
			muteAllSFXMusic();
		else 
			unMuteAllCurrentSFX();
	}
	
	private void unMuteAllCurrentSFX() {
		
		foreach(GameObject music in soundSFX.Values){
			if(music.GetComponent<AudioSource>())
				music.GetComponent<AudioSource>().mute = false;
		}
	}
	
	private void unMuteAllCurrentBGMusic() {
		foreach(GameObject music in soundBG.Values){
			if(music.GetComponent<AudioSource>())
				music.GetComponent<AudioSource>().mute = false;
		}
		
	}
	
	private void muteAllBGMusic() {
		foreach(GameObject music in soundBG.Values){
			if(music.GetComponent<AudioSource>() != null)
				music.GetComponent<AudioSource>().mute = true;
		}
		
	}
	
	private void muteAllSFXMusic() {
		foreach(GameObject music in soundSFX.Values){
			if(music.GetComponent<AudioSource>())
				music.GetComponent<AudioSource>().mute = true;
		}
	}


    public float MinSameSFXSoundDelaySec = 0.1f;

    public float VolumeDurationSec = 1f;

    public void SetStopCurrentBackGroundSound()
    {
        if (currentThemeSound != SOUND_NAME.None)
        {
            if (soundBG[currentThemeSound] != null)
            {
                GameObject sound = (GameObject)soundBG[currentThemeSound];
                if (sound != null)
                {
                    sound.GetComponent<AudioSource>().DOFade(0f, VolumeDurationSec)
                        .OnComplete(() => { sound.GetComponent<AudioSource>().Stop(); });
                }
                else
                {
                    soundBG.Remove(currentThemeSound);
                }
            }
        }

    }
    public void SetPlayCurrentBackGroundSound()
    {
        playSoundBackGround(currentThemeSound);
    }

	public GameObject playSoundBackGround(SOUND_NAME type, bool p_direct = false) 
    {
        if (!isBGMusicOn)
            return null;


		GameObject retVal = null;

		if(currentThemeSound == type
            && !p_direct )
			return null;

		if(soundBG[type] != null) {
			GameObject sound = (GameObject) soundBG[type];	
            AudioSource audiosourceobj = sound.GetComponent<AudioSource>();
			if(sound != null) {
				sound.GetComponent<AudioSource>().Play();
				sound.GetComponent<AudioSource>().volume = 0f;
                sound.GetComponent<AudioSource>().DOFade(1f, VolumeDurationSec)
                    .OnComplete(() => 
                    {
                        if ( !audiosourceobj.isPlaying )
                        {
                            Debug.LogErrorFormat("Delay Sound Fail RePlay : ");
                            audiosourceobj.Play();
                        }
                    }
                    );

				retVal = sound;
			}
			else {
				soundBG.Remove(type);
				
			}
		}
		
		if(retVal == null) 
        {
			AudioClip clipAudio = getSound(type);
			
			GameObject go = new GameObject("AudioSource", typeof(AudioSource));
			AudioSource audioSource = go.GetComponent<AudioSource>();
			go.transform.parent = transform;
			go.name = type.ToString();
			soundBG.Add(type, go);

			audioSource.clip = clipAudio;
			audioSource.loop = true;
			audioSource.spatialBlend = 0.0f;
			//audioSource.volume = 1f;
			audioSource.Play();
            audioSource.volume = 0f;
            audioSource.DOFade(1f, VolumeDurationSec);


			retVal = go;
		}

		if(currentThemeSound != SOUND_NAME.None) {
			if(soundBG[currentThemeSound] != null) {
				GameObject sound = (GameObject) soundBG[currentThemeSound];	
				if(sound != null) {
					//sound.GetComponent<AudioSource>().Stop();
                    //audioSource.volume = 0f;
                    sound.GetComponent<AudioSource>().DOFade(0f, VolumeDurationSec)
                        .OnComplete(() => { sound.GetComponent<AudioSource>().Stop(); });
				}
				else {
					soundBG.Remove(currentThemeSound);
				}
			}
		}

		currentThemeSound = type;
		recheckAllSound();

		return retVal;
	}

	void OnApplicationPause(bool pause) {
		Debug.Log("SoundManager Pause : " + pause);
		if(pause) {
			turnOffSoundTemporary();
		}
		else {
			recoverSoundState();
		}
	}

    

    public void ShotSoundSFX(SOUND_NAME p_type)
    {
        if (!isSFXOn)
            return;

        string shotplayurl = m_AudioClipURLDic[p_type].ShotURL;

        
        if ( !m_AudioClipStartTime.ContainsKey(p_type) )
        {
            m_AudioClipStartTime.Add(p_type, 0f);
            Debug.LogErrorFormat("ClipStart Null : {0}", p_type);
        }

        float lastsec = m_AudioClipStartTime[p_type] + MinSameSFXSoundDelaySec;
        float currentsec = Time.time;
        if (lastsec > currentsec)
        {
            Debug.LogFormat("SameDupuleSound : {0}", p_type);
            return;
        }


        //Debug.LogFormat("Shot Sound Play : {0}", p_type);
        m_AudioClipStartTime[p_type] = Time.time;
        AudioCenter.playSound(shotplayurl);
    }

	public GameObject playSoundSFX(SOUND_NAME type) 
    {

		//		Debug.Log("playSound : " + type);
		GameObject retVal = null ;
		bool flag = true;
		
		if(soundSFX[type] != null) {
			GameObject sound = (GameObject) soundSFX[type];	
			if(sound != null) {
				sound.GetComponent<AudioSource>().Play();
				retVal = sound;
				//			Debug.Log("Play 1");
				flag = false;
			}
			else {
		 		soundSFX.Remove(type);
				
			}
		}
		
		if(flag) {
			AudioClip sound = getSound(type);
			
			GameObject audioSource = new GameObject("AudioSource", typeof(AudioSource));
			audioSource.transform.parent = transform;
			audioSource.GetComponent<AudioSource>().clip = sound;
			audioSource.name = type.ToString();
			soundSFX.Add(type, audioSource);
			audioSource.GetComponent<AudioSource>().spatialBlend = 0.0f;
			audioSource.GetComponent<AudioSource>().Play();
			
			retVal = audioSource;
		}
		
		recheckAllSound();
		return retVal;
	}





    public class SoundLinkStringName
    {
        public string ClipURL = "";
        public string ShotURL = "";
    }

    public static Dictionary<SOUND_NAME, float> m_AudioClipStartTime = new Dictionary<SOUND_NAME, float>((int)SOUND_NAME.None);
    public static Dictionary<SOUND_NAME, SoundLinkStringName> m_AudioClipURLDic = new Dictionary<SOUND_NAME, SoundLinkStringName>((int)SOUND_NAME.None)
    {
        // 기존 사운드
        //{ SOUND_NAME.Shoot, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/shoot", ShotURL="shoot"} } 
        { SOUND_NAME.Shoot, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/Kick_Sound", ShotURL="Kick_Sound"} } 
        , { SOUND_NAME.GoalKeeper_Catch, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/goalkeeper_catch", ShotURL="goalkeeper_catch"} } 
        , { SOUND_NAME.Whistle, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/whistle", ShotURL="whistle"} } 
        , { SOUND_NAME.Crowd_Goal, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/crowd_goal", ShotURL="crowd_goal"} } 
        , { SOUND_NAME.Crowd_Out, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/crowd_out", ShotURL="crowd_out"} } 
        , { SOUND_NAME.Button, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/button", ShotURL="button"} } 
        , { SOUND_NAME.Ball_Hit_Bar, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/ball_hit_bar 1", ShotURL="ball_hit_bar 1"} } 
        , { SOUND_NAME.Ball_Hit_Net, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_NetHit02", ShotURL="InGame_NetHit02"} } 
        , { SOUND_NAME.Ball_Hit_Player_Wall, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/goalkeeper_catch", ShotURL="goalkeeper_catch"} } 
        , { SOUND_NAME.Ball_Hit_Goal, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/ball_hit_goal_signal", ShotURL="ball_hit_goal_signal"} } 
        , { SOUND_NAME.Ball_Hit_Goal_Extra, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/ball_hit_goal_extra_signal", ShotURL="ball_hit_goal_extra_signal"} } 
        , { SOUND_NAME.BG_Crowd, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/crowd_out", ShotURL="crowd_out"} } 
        , { SOUND_NAME.BG_Menu, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/BG_Football_Menu", ShotURL="BG_Football_Menu"} } 
        , { SOUND_NAME.BG_Menu1, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/BG_Football_Fast", ShotURL="BG_Football_Fast"} } 

        // 추가된 shot사운드
        , { SOUND_NAME.Keeper_Punch, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/Keeper_Punch_01", ShotURL="Keeper_Punch_01"} } 
        , { SOUND_NAME.InGame_StartCount, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_StartCount", ShotURL="InGame_StartCount"} } 
        , { SOUND_NAME.InGame_StartWhistle, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_StartWhistle", ShotURL="InGame_StartWhistle"} } 
        , { SOUND_NAME.InGame_EndWhistle, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_EndWhistle", ShotURL="InGame_EndWhistle"} } 
        , { SOUND_NAME.InGame_GoalPostHit, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/ball_hit_bar", ShotURL="ball_hit_bar"} } 
        , { SOUND_NAME.InGame_BallShootWind0, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_BallShootWind0", ShotURL="InGame_BallShootWind0"} } 
        , { SOUND_NAME.InGame_NetHit0, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_NetHit02", ShotURL="InGame_NetHit02"} } 
        , { SOUND_NAME.InGame_NetHit1, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_NetHit02", ShotURL="InGame_NetHit02"} } 
        , { SOUND_NAME.InGame_Sighs, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_Sighs", ShotURL="InGame_Sighs"} } 
        , { SOUND_NAME.InGame_PantingMale, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_PantingMale", ShotURL="InGame_PantingMale"} } 

        // 관중석 랜덤 소리
        , { SOUND_NAME.InGame_BuBuJella, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_BuBuJella", ShotURL="InGame_BuBuJella"} } 
        , { SOUND_NAME.InGame_Scrim00, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_Scrim00", ShotURL="InGame_Scrim00"} } 
        , { SOUND_NAME.InGame_Scrim01, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_Scrim01", ShotURL="InGame_Scrim01"} } 
        , { SOUND_NAME.InGame_Scrim02, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_Scrim02", ShotURL="InGame_Scrim02"} } 
        , { SOUND_NAME.InGame_Scrim03, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_Scrim03", ShotURL="InGame_Scrim03"} } 
        , { SOUND_NAME.InGame_Scrim04, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_Scrim04", ShotURL="InGame_Scrim04"} } 

        // 폭죽소리
        , { SOUND_NAME.InGame_FireCracker, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/InGame_Firecracker_Medium", ShotURL="InGame_Firecracker_Medium"} } 

        // 그라운드충돌
        , { SOUND_NAME.InGame_GroundHit0, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/Ground01", ShotURL="Ground01"} } 
        , { SOUND_NAME.InGame_GroundHit1, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/Ground02", ShotURL="Ground02"} } 
        , { SOUND_NAME.InGame_GroundHit2, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/Ground03", ShotURL="Ground03"} } 

        // 펜스
        , { SOUND_NAME.InGame_PenceHit0, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/Pence00", ShotURL="Pence00"} } 
        , { SOUND_NAME.InGame_PenceHit1, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/Pence01", ShotURL="Pence01"} } 
        , { SOUND_NAME.InGame_PenceHit2, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/Pence02", ShotURL="Pence02"} } 

        , { SOUND_NAME.InGame_MultiOtherAddScore, new SoundLinkStringName(){ClipURL = "Sound/ShotSound/ball_hit_goal_signal", ShotURL="ball_hit_goal_signal"} } 

        // BGM
        , { SOUND_NAME.MainUIBGM0, new SoundLinkStringName(){ClipURL = "Sound/BGM/LobbyMusic", ShotURL=""} } 
        , { SOUND_NAME.MainUIBGM1, new SoundLinkStringName(){ClipURL = "Sound/BGM/LobbyMusic1", ShotURL=""} } 
        , { SOUND_NAME.InGameBGM0, new SoundLinkStringName(){ClipURL = "Sound/BGM/InGame_BGM0", ShotURL=""} } 
        , { SOUND_NAME.InGameBGM1, new SoundLinkStringName(){ClipURL = "Sound/BGM/InGame_BGM1", ShotURL=""} } 
        , { SOUND_NAME.InGame_MissionSuccess, new SoundLinkStringName(){ClipURL = "Sound/BGM/InGame_MissionSuccess", ShotURL=""} } 
        , { SOUND_NAME.InGame_MissionFail, new SoundLinkStringName(){ClipURL = "Sound/BGM/InGame_BGM0", ShotURL=""} } 


    };
    protected static Dictionary<SOUND_NAME, AudioClip> m_AudioClipDic = new Dictionary<SOUND_NAME, AudioClip>();
    public static AudioClip GetSoundClipDic(SOUND_NAME p_type)
    {
        if ( m_AudioClipDic.ContainsKey(p_type) )
        {
            return m_AudioClipDic[p_type];
        }
        
        AudioClip outclip = Resources.Load(m_AudioClipURLDic[p_type].ClipURL, typeof(AudioClip)) as AudioClip;
        if (outclip != null)
        {
            m_AudioClipDic.Add(p_type, outclip );
        }

        return outclip;
    }

    


	public static AudioClip getSound(SOUND_NAME type) 
    {
		string name = "";
// 		switch (type) {
// 		case SOUND_NAME.Shoot:
// 			name = "Sound/shoot";
// 			break;
// 		case SOUND_NAME.GoalKeeper_Catch:
// 			name = "Sound/goalkeeper_catch";
// 			break;
// 		case SOUND_NAME.Whistle:
// 			name = "Sound/whistle";
// 			break;
// 		case SOUND_NAME.Crowd_Goal:
// 			name = "Sound/crowd_goal";
// 			break;
// 		case SOUND_NAME.Crowd_Out:
// 			name = "Sound/crowd_out";
// 			break;
// 		case SOUND_NAME.Button:
// 			name = "Sound/button";
// 			break;
// 		case SOUND_NAME.Ball_Hit_Bar:
// 			name = "Sound/ball_hit_bar";
// 			break;
// 		case SOUND_NAME.Ball_Hit_Net:
// 			name = "Sound/ball_hit_net";
// 			break;
// 		case SOUND_NAME.Ball_Hit_Player_Wall:
// 			name = "Sound/ball_hit_player_wall";
// 			break;
// 		case SOUND_NAME.Ball_Hit_Goal:
// 			name = "Sound/ball_hit_goal_signal";
// 			break;
// 		case SOUND_NAME.Ball_Hit_Goal_Extra:
// 			name = "Sound/ball_hit_goal_extra_signal";
// 			break;
// 		case SOUND_NAME.BG_Crowd:
// 			name = "Sound/BG_crowd";
// 			break;
// 		case SOUND_NAME.BG_Menu:
// 			name = "Sound/BG_Football_Menu";
// 			break;
// 		case SOUND_NAME.BG_Menu1:
// 			name = "Sound/BG_Football_Fast";
// 			break;
// 
// 		default:
// 			break;
// 		}



        name = m_AudioClipURLDic[type].ClipURL;

		return  Resources.Load(name, typeof(AudioClip)) as AudioClip;
	}


//     public void onClick_Button()
//     {
//         playSoundSFX(SOUND_NAME.Button);
//     }


    public void OnMainLogoSoundCI()
    {
        if (SoundManager.share == null)
            return;

//         if (!isSFXOn)
//             return;

        AudioCenter.playSound("SOUND_ci");
    }
    public void OnScoreAddSound()
    {
        if (SoundManager.share == null)
            return;

        if (!isSFXOn)
            return;

        AudioCenter.playSound("ScoreCount");
    }
    public void OnStarSound()
    {
        if (SoundManager.share == null)
            return;

        if (!isSFXOn)
            return;

        AudioCenter.playSound("SOUND_RESULT_STAR");
    }

    public void OnMenuMoveSound()
    {
        if (SoundManager.share == null )
            return;

        if (!isSFXOn)
            return;

        AudioCenter.playSound("menu move");
    }

    public void OnClick_ButtonType( E_ButtonSoundType p_btntype, string p_targetname )
    {
        if (!isSFXOn)
            return;

        switch (p_btntype)
        {
            case E_ButtonSoundType.OKBTN:
                AudioCenter.playSound("Click02");
                break;
            case E_ButtonSoundType.Cancel:
                AudioCenter.playSound("Click01");
                break;
            case E_ButtonSoundType.Back:
                AudioCenter.playSound("Click01");
                break;
            case E_ButtonSoundType.Ignore:
                AudioCenter.playSound("Click01");
                break;
            case E_ButtonSoundType.Next:
                AudioCenter.playSound("Click02");
                break;
            case E_ButtonSoundType.MenuOpen:
                AudioCenter.playSound("OpenMenu");
                break;
            case E_ButtonSoundType.Retry:
                AudioCenter.playSound("Click02");
                break;
            case E_ButtonSoundType.DefaultSkill:
                AudioCenter.playSound("Click01");
                break;
            case E_ButtonSoundType.BallSkill:
                AudioCenter.playSound("Click01");
                break;
            case E_ButtonSoundType.Click0:
                AudioCenter.playSound("Click03");
                break;
            case E_ButtonSoundType.Click1:
                AudioCenter.playSound("Click04");
                break;
            case E_ButtonSoundType.Click2:
                AudioCenter.playSound("Click03");
                break;
            case E_ButtonSoundType.Click3:
                AudioCenter.playSound("Click04");
                break;
            case E_ButtonSoundType.None:
            case E_ButtonSoundType.Max:
            default:
                {
                    Debug.LogFormat("None Sound : {0}, {1}"
                        , p_btntype
                        , p_targetname );
                }
                break;
        }

    }


    [Header("[랜덤인게임 사운드범위]")]
    [SerializeField]
    bool m_ISRandomScrimShotPlay = false;
    public float RandomMinSec = 0.4f;
    public float RandomMaxSec = 4f;
    public void SetRandomScrimeShotPlay(bool p_flag)
    {
        m_ISRandomScrimShotPlay = p_flag;

    }

    void UpdateRandomScrime()
    {
        float currsec = Time.time;
        if (m_RandomPlaySec <= currsec )
        {
            m_RandomPlaySec = currsec + UnityEngine.Random.Range(RandomMinSec, RandomMaxSec);
            PlayRandomInGameScrim();
        }
    }

    float m_RandomPlaySec = 0f;
    void Update()
    {
        if (m_ISRandomScrimShotPlay)
        {
            UpdateRandomScrime();
        }

    }

    public void PlayRandomInGameScrim()
    {
        SOUND_NAME randomingamescrime = (SOUND_NAME)UnityEngine.Random.Range((int)SOUND_NAME.InGame_BuBuJella, (int)SOUND_NAME.InGame_Scrim04 + 1);
        ShotSoundSFX(randomingamescrime);
        //playSoundSFX(randomingamescrime);


    }




    void BallEvent_OnCollisionEnter(Collision p_other)
    {

        string tag = "";
        if (!string.IsNullOrEmpty(p_other.gameObject.tag))
        {
            tag = p_other.gameObject.tag;
        }


        if (tag.Equals("Wall"))
        {
            ShotSoundSFX(SOUND_NAME.Ball_Hit_Player_Wall);
        }

        if (tag.Equals("Net"))
        {
            ShotSoundSFX(SOUND_NAME.Ball_Hit_Net);
        }

        if (tag.Equals("ground"))
        {
            SOUND_NAME rand = (SOUND_NAME)UnityEngine.Random.Range((int)SOUND_NAME.InGame_GroundHit0, (int)SOUND_NAME.InGame_GroundHit2 + 1);
            ShotSoundSFX(rand);
        }

        if (tag.Equals("Pence"))
        {
            SOUND_NAME rand = (SOUND_NAME)UnityEngine.Random.Range((int)SOUND_NAME.InGame_PenceHit0, (int)SOUND_NAME.InGame_PenceHit2 + 1);
            ShotSoundSFX(rand);
        }



        //string tag = other.gameObject.tag;
//         if (tag.Equals("Player") || tag.Equals("Obstacle") || tag.Equals("Net") || tag.Equals("Wall"))
//         {	// banh trung thu mon hoac khung thanh hoac da vao luoi roi thi ko cho banh bay voi van toc nua, luc nay de~ cho physics engine tinh' toan' quy~ dao bay
//             
//         }
    }







// 
//     public void OnClick_OKButton()
//     {
//         if (isSFXOn)
//             AudioCenter.playSound("Click02");
//     }
// 
//     public void OnClick_CancelButton()
//     {
//         if (isSFXOn)
//             AudioCenter.playSound("Click01");
//     }
// 
//     public void OnClick_BackButton()
//     {
//         if (isSFXOn)
//             AudioCenter.playSound("Click01");
//     }
// 
//     public void OnClick_RetryButton()
//     {
//         if (isSFXOn)
//             AudioCenter.playSound("Click01");
//     }
// 
//     public void OnClick_DefaultSkill()
//     {
//         if (isSFXOn)
//             AudioCenter.playSound("Click01");
//     }
//     public void OnClick_BallSkill()
//     {
//         if (isSFXOn)
//             AudioCenter.playSound("Click01");
//     }
//     public void OnClick_Click()
//     {
//         if (isSFXOn)
//             AudioCenter.playSound("Click01");
//     }
// 
//     public void OnMenuOpen()
//     {
//         if (isSFXOn)
//             AudioCenter.playSound("OpenMenu");
//     }




}
