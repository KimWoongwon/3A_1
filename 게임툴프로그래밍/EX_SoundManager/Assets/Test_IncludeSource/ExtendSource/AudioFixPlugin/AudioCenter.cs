/*
 * 
 * 
 *
using UnityEngine;
using System.Collections.Generic;
using System.Collections;

public class AudioCenter_Temp : MonoBehaviour {
	private static AudioCenter_Temp instance;
	private AudioSource audioSource;

	#if UNITY_ANDROID && !UNITY_EDITOR
		public static AndroidJavaClass unityActivityClass ;
		public static AndroidJavaObject activityObj ;
		private static AndroidJavaObject soundObj ;
		
		public static void playSound( int soundId ) {
			soundObj.Call( "playSound", new object[] { soundId } );
		}
		
		public static void playSound( int soundId, float volume ) {
			soundObj.Call( "playSound", new object[] { soundId, volume } );
		}
		
		public static void playSound( int soundId, float leftVolume, float rightVolume, int priority, int loop, float rate  ) {
			soundObj.Call( "playSound", new object[] { soundId, leftVolume, rightVolume, priority, loop, rate } );
		}
		
		public static int loadSound( string soundName ) {
			return soundObj.Call<int>( "loadSound", new object[] { "Resources/Sounds/" +  soundName + ".wav" } );
		}
		
		public static void unloadSound( int soundId ) {
			soundObj.Call( "unloadSound", new object[] { soundId } );
		}
	#else
		private Dictionary<int, AudioClip> audioDic = new Dictionary<int, AudioClip>();
		
		public static void playSound( int soundId ) {
			//AudioCenter_Temp.instance.audioSource.clip = AudioCenter_Temp.instance.audioDic[soundId];
			AudioCenter_Temp.instance.audioSource.PlayOneShot(AudioCenter_Temp.instance.audioDic[soundId]);
		}

		public static void playSound( int soundId, float volume ) {
			AudioCenter_Temp.instance.audioSource.PlayOneShot(AudioCenter_Temp.instance.audioDic[soundId], volume);
		}

		public static void playSound( int soundId, float leftVolume, float rightVolume, int priority, int loop, float rate ) {
			//float panRatio = AudioCenter_Temp.instance.audioSource.panStereo;
			//rightVolume = Mathf.Clamp(rightVolume, 0, 1);
			//leftVolume = Mathf.Clamp(leftVolume, 0, 1);
			//AudioCenter_Temp.instance.audioSource.panStereo = Mathf.Clamp(rightVolume, 0, 1) - Mathf.Clamp(leftVolume, 0, 1);
			float volume = (leftVolume + rightVolume) / 2;
			AudioCenter_Temp.instance.audioSource.PlayOneShot(AudioCenter_Temp.instance.audioDic[soundId], volume);
			//AudioCenter_Temp.instance.audioSource.panStereo = panRatio;
		}
		
		public static int loadSound( string soundName ) {
			var soundID = soundName.GetHashCode();
			var audioClip = Resources.Load<AudioClip>("Sounds/" + soundName);
			AudioCenter_Temp.instance.audioDic[soundID] = audioClip;
			
			return soundID;
		}
		
		public static void unloadSound( int soundId ) {
			var audioClip = AudioCenter_Temp.instance.audioDic[soundId];
			Resources.UnloadAsset(audioClip);
			AudioCenter_Temp.instance.audioDic.Remove(soundId);
		}
	#endif

	private void Awake() {
		if (instance == null || instance == this) {
			instance = this;
		} else {
			Destroy(this);
			return;
		}
		
		#if !UNITY_ANDROID || UNITY_EDITOR
			audioSource = gameObject.AddComponent<AudioSource>();
			audioSource.hideFlags = HideFlags.HideInInspector;
		#else
			unityActivityClass =  new AndroidJavaClass( "com.unity3d.player.UnityPlayer" );
			activityObj = unityActivityClass.GetStatic<AndroidJavaObject>( "currentActivity" );
			//soundObj = new AndroidJavaObject( "com.catsknead.androidsoundfix.AudioCenter_Temp", 1, activityObj, activityObj );
			soundObj = new AndroidJavaObject( "com.catsknead.androidsoundfix.AudioCenter_Temp", 5, activityObj );
		#endif
	}
}




 */















using UnityEngine;
using System.Collections.Generic;
using System.Collections;
using System.IO;
#if UNITY_EDITOR
using UnityEditor;
#endif

public class AudioCenter : MonoBehaviour {
	private static AudioCenter instance;
    [SerializeField]
	protected AudioSource audioSource;



    // 복사두기
    // Plugins/Android/AudioFixAndroidCatsknead.jar


    // 뒤에 / 넣어야지됨
    [SerializeField]
    protected string m_DefaultSoundFolder = "Resources/";
    // 뒤에 / 넣어야지됨
    [SerializeField]
    protected string m_ExtendSoundFolder = "ShotSound/";

    // . 없이 확장자만 적으면됨
    [SerializeField]
    protected string m_DefaultFileExtend = "wav";

    protected string m_FolderPath = "";

//     public string DefaultSoundFolder
//     {
//         get { return m_DefaultSoundFolder; }
//         set { m_DefaultSoundFolder = value; }
//    

    [SerializeField]
    protected Dictionary<string, int> m_SyncStringNInt = new Dictionary<string, int>();




    private void Editor_CopyAdjustSoundDatas()
    {
#if UNITY_EDITOR
        string resourcepath = string.Format("{0}/{1}{2}"
            , Application.dataPath
            , m_DefaultSoundFolder
            , m_ExtendSoundFolder );

        string streamassertpath = string.Format("{0}/StreamingAssets/{1}{2}"
            , Application.dataPath
            , m_DefaultSoundFolder
            , m_ExtendSoundFolder );

        Debug.LogFormat("Copy SoundFile : {0}, {1}", resourcepath, streamassertpath );
        //FileUtil.CopyFileOrDirectory(resourcepath, streamassertpath);
        Editor_CopyAllFileNFolder(resourcepath, streamassertpath, true);
#endif

    }

    private void Editor_CopyAllFileNFolder( string p_resourcepath, string p_destpath, bool p_copysubdir )
    {

#if UNITY_EDITOR
//         string resourcepath = string.Format("{0}/{1}{2}"
//             , Application.dataPath
//             , m_DefaultSoundFolder
//             , m_ExtendSoundFolder);
// 
//         string streamassertpath = string.Format("{0}/StreamingAssets/{1}"
//             , Application.dataPath
//             , m_ExtendSoundFolder);
// 
// 
//         string sourceDirName = resourcepath;
//         string destDirName = streamassertpath;


        // Get the subdirectories for the specified directory.
        DirectoryInfo dir = new DirectoryInfo(p_resourcepath);

        if (!dir.Exists)
        {
            throw new DirectoryNotFoundException(
                "Source directory does not exist or could not be found: "
                + p_resourcepath);
        }

        DirectoryInfo[] dirs = dir.GetDirectories();
        // If the destination directory doesn't exist, create it.
        if (!Directory.Exists(p_destpath))
        {
            Directory.CreateDirectory(p_destpath);
        }

        // 모든 확장자가 복사됨 wav만 적용할지등 작업 해주기
        // Get the files in the directory and copy them to the new location.
        FileInfo[] files = dir.GetFiles();
        foreach (FileInfo file in files)
        {
            string temppath = Path.Combine(p_destpath, file.Name);
            file.CopyTo(temppath, true);
        }

        // If copying subdirectories, copy them and their contents to new location.
        if (p_copysubdir)
        {
            foreach (DirectoryInfo subdir in dirs)
            {
                string temppath = Path.Combine(p_destpath, subdir.Name);
                Editor_CopyAllFileNFolder(subdir.FullName, temppath, p_copysubdir);
            }
        }

#endif
    }


#if UNITY_EDITOR
    [ContextMenu("[사운드파일폴더정리 -> Wav파일만있는지 확인하기 ]")]
    private void Editor_AdjustResourceFolderDatas()
    {
        string filepath = string.Format("{0}"
            , m_ExtendSoundFolder);

        Debug.LogFormat("Shot Sound : {0}", m_ExtendSoundFolder);
        BeepAudioClipArr = Resources.LoadAll<AudioClip>(m_ExtendSoundFolder);

        Editor_CopyAdjustSoundDatas();
        Debug.LogFormat("Shot Sound Load 2 : {0}", BeepAudioClipArr.Length);
    }
#endif






#if UNITY_ANDROID && !UNITY_EDITOR
    //#if UNITY_EDITOR
		public static AndroidJavaClass unityActivityClass ;
		public static AndroidJavaObject activityObj ;
		private static AndroidJavaObject soundObj ;
		
		public static void playSound( int soundId ) {
			soundObj.Call( "playSound", new object[] { soundId } );
		}
	
	    public static void playSound( string p_soundname ) 
        {
            int soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "playSound", new object[] { soundId } );
		}

		public static void playSound( int soundId, float volume ) {
			soundObj.Call( "playSound", new object[] { soundId, volume } );
		}

        public static void playSound( string p_soundname, float volume ) 
        {
            int soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "playSound", new object[] { soundId, volume } );
		}
		
		public static void playSound( int soundId, float leftVolume, float rightVolume, int priority, int loop, float rate  ) {
			soundObj.Call( "playSound", new object[] { soundId, leftVolume, rightVolume, priority, loop, rate } );
		}
		

        public static void playSound( string p_soundname, float leftVolume, float rightVolume, int priority, int loop, float rate  ) 
        {
            int soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "playSound", new object[] { soundId, leftVolume, rightVolume, priority, loop, rate } );
		}

		public static int loadSound( string soundName ) 
        {
            string fullname = string.Format("{0}{1}.{2}"
                , AudioCenter.instance.m_FolderPath
                , soundName
                , AudioCenter.instance.m_DefaultFileExtend);
            int soundID = soundObj.Call<int>( "loadSound", new object[] { fullname } );
            AudioCenter.instance.m_SyncStringNInt.Add(soundName, soundID);
			return soundID;
		}
		
        public static void unloadSound( string p_soundname ) 
        {
            int soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "unloadSound", new object[] { soundId } );

            AudioCenter.instance.m_SyncStringNInt.Remove(p_soundname);
		}

		public static void unloadSound( int soundId ) {
			soundObj.Call( "unloadSound", new object[] { soundId } );
		}
#else
    private Dictionary<int, AudioClip> audioDic = new Dictionary<int, AudioClip>();
	

	public static void playSound( int soundId ) {
		AudioCenter.instance.audioSource.PlayOneShot(AudioCenter.instance.audioDic[soundId]);
	}

    public static void playSound(string p_soundname)
    {
        int soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
        AudioCenter.instance.audioSource.PlayOneShot(AudioCenter.instance.audioDic[soundId]);
    }

	public static void playSound( int soundId, float volume ) {
		AudioCenter.instance.audioSource.PlayOneShot(AudioCenter.instance.audioDic[soundId], volume);
	}

    public static void playSound(string p_soundname, float volume)
    {
        int soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
        AudioCenter.instance.audioSource.PlayOneShot(AudioCenter.instance.audioDic[soundId], volume);
    }

	public static void playSound( int soundId, float leftVolume, float rightVolume, int priority, int loop, float rate ) {
		//float panRatio = AudioCenter.instance.audioSource.panStereo;
		//rightVolume = Mathf.Clamp(rightVolume, 0, 1);
		//leftVolume = Mathf.Clamp(leftVolume, 0, 1);
		//AudioCenter.instance.audioSource.panStereo = Mathf.Clamp(rightVolume, 0, 1) - Mathf.Clamp(leftVolume, 0, 1);
		float volume = (leftVolume + rightVolume) / 2;
		AudioCenter.instance.audioSource.PlayOneShot(AudioCenter.instance.audioDic[soundId], volume);
		//AudioCenter.instance.audioSource.panStereo = panRatio;
	}
        

    public static void playSound( string p_soundname, float leftVolume, float rightVolume, int priority, int loop, float rate )
    {
        int soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
        float volume = (leftVolume + rightVolume) / 2;
        AudioCenter.instance.audioSource.PlayOneShot(AudioCenter.instance.audioDic[soundId], volume);
    }


		
//         public static void SetSoundFolder(string p_folderurl)
//         {
//             AudioCenter.instance.m_DefaultSoundFolder = p_folderurl;
//         }

    public static int loadSound( string soundName )
    {
		int soundID = soundName.GetHashCode();
        string fullname = string.Format("{0}{1}", AudioCenter.instance.m_ExtendSoundFolder, soundName);
        //AudioClip audioClip = Resources.Load<AudioClip>("Sounds/" + soundName);
        //AudioClip audioClip = Resources.Load<AudioClip>(fullname);
        //AudioCenter.instance.audioDic[soundID] = audioClip;
        //AudioCenter.instance.m_SyncStringNInt[soundName] = soundID;
            
        AudioClip audioClip = null;



        if (audioClip == null)
        {
            Debug.LogErrorFormat(" Audio Sound NullData : {0}", fullname);
        }

        AudioCenter.instance.audioDic.Add(soundID, audioClip);
        AudioCenter.instance.m_SyncStringNInt.Add(soundName, soundID);




        string tempfullname = string.Format("{0}{1}.{2}"
            , AudioCenter.instance.m_FolderPath
            , soundName
            , AudioCenter.instance.m_DefaultFileExtend);

        Debug.Log("Test FullName : " + tempfullname );

		return soundID;
	}
		
	public static void unloadSound( int soundId ) {
		var audioClip = AudioCenter.instance.audioDic[soundId];
		Resources.UnloadAsset(audioClip);
		AudioCenter.instance.audioDic.Remove(soundId);
	}

    public static void unloadSound(string p_soundname)
    {
        int soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
        var audioClip = AudioCenter.instance.audioDic[soundId];
        Resources.UnloadAsset(audioClip);
        AudioCenter.instance.audioDic.Remove(soundId);
        AudioCenter.instance.m_SyncStringNInt.Remove(p_soundname);
    }

	#endif

	private void Awake() {
		if (instance == null || instance == this) {
			instance = this;
		} else {
			Destroy(this);
			return;
		}

        m_FolderPath = string.Format("{0}{1}", m_DefaultSoundFolder, m_ExtendSoundFolder);

		#if !UNITY_ANDROID || UNITY_EDITOR
            if (audioSource == null)
            {
                audioSource = gameObject.AddComponent<AudioSource>();
                //audioSource.hideFlags = HideFlags.HideInInspector;
            }
			
		#else
			unityActivityClass =  new AndroidJavaClass( "com.unity3d.player.UnityPlayer" );
			activityObj = unityActivityClass.GetStatic<AndroidJavaObject>( "currentActivity" );
			//soundObj = new AndroidJavaObject( "com.catsknead.androidsoundfix.AudioCenter", 1, activityObj, activityObj );
			soundObj = new AndroidJavaObject( "com.catsknead.androidsoundfix.AudioCenter", 5, activityObj );
		#endif

        InitDefaultSoundLoader();
	}


    protected void InitDefaultSoundLoader()
    {

#if UNITY_EDITOR
        int count = BeepAudioClipArr.Length;
        for (int i = 0; i < count; i++)
        {
            Debug.Log(" Init Loader Audio clip name : " + BeepAudioClipArr[i].name);
            AudioCenter.loadSound(BeepAudioClipArr[i].name);
        }
#endif

    }

#if UNITY_ANDROID && !UNITY_EDITOR

#else
    [Header("[DefaultBeepSound]")]
    public AudioClip[] BeepAudioClipArr;
#endif
    
    public void OnPlaySound(AudioClip p_audioclip)
    {
        if (m_SyncStringNInt.ContainsKey(p_audioclip.name) )
        {
            AudioCenter.playSound(p_audioclip.name);
        }
        else
        {
            Debug.LogError("Miss Matching AudioClip : ");
        }
        
    }

    public static void PlaySound(AudioClip p_audioclip)
    {
        if (AudioCenter.instance.m_SyncStringNInt.ContainsKey(p_audioclip.name))
        {
            AudioCenter.playSound(p_audioclip.name);
        }
        else
        {
            Debug.LogError(" Static PlaySound Miss Matching AudioClip : ");
        }
    }

    public static void PlaySound(string p_audioclip)
    {
        if (AudioCenter.instance.m_SyncStringNInt.ContainsKey(p_audioclip))
        {
            AudioCenter.playSound(p_audioclip);
        }
        else
        {
            Debug.LogError(" Static PlaySound Miss Matching AudioClip Str : ");
        }
    }

}
