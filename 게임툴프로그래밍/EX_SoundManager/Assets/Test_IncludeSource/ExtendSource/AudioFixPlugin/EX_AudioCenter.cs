using UnityEngine;
using System.Collections.Generic;
using System.Collections;
using System.IO;
#if UNITY_EDITOR
using UnityEditor;
#endif


[System.Serializable]
public class Editor_EXShotAudioClipLabelNFolder
{
    /// <summary>
    /// 폴더 사용위한 기본 라벨
    /// </summary>
    public string LabelName = "";
    /// <summary>
    /// 참고할 사운드 폴더
    /// </summary>
    public string SoundFolder = "";
    /// <summary>
    /// 참고용 확장자
    /// </summary>
    public string DefaultFileExtend = "wav";
}

public class EX_AudioCenter : MonoBehaviour
{
    private static EX_AudioCenter instance;
    [SerializeField]
    protected AudioSource audioSource;

    
    
    // 뒤에 / 넣어야지됨
    [SerializeField]
    protected string m_DefaultSoundFolder = "Resources/";
    public string DefaultSoundFolder
    {
        get { return m_DefaultSoundFolder; }
        protected set { m_DefaultSoundFolder = value; }
    }

    // 뒤에 / 넣어야지됨
    [SerializeField]
    protected string m_ExtendSoundFolder = "ShotSound/";
    public string ExtendSoundFolder
    {
        get { return m_ExtendSoundFolder; }
        protected set { m_ExtendSoundFolder = value; }
    }

    // . 없이 확장자만 적으면됨
    [SerializeField]
    protected string m_DefaultFileExtend = "wav";
    public string DefaultFileExtend
    {
        get { return m_DefaultFileExtend; }
        protected set { m_DefaultFileExtend = value; }
    }
    
    public List<Editor_EXShotAudioClipLabelNFolder> ExShotAudioClipList = new List<Editor_EXShotAudioClipLabelNFolder>();



    protected string m_FolderPath = "";
    
    protected Dictionary<string, int> m_SyncStringNInt = new Dictionary<string, int>();
    

    private void Editor_CopyAdjustSoundDatas()
    {
#if UNITY_EDITOR
        string resourcepath = string.Format("{0}/{1}{2}"
            , Application.dataPath
            , m_DefaultSoundFolder
            , m_ExtendSoundFolder);

        string streamassertpath = string.Format("{0}/StreamingAssets/{1}{2}"
            , Application.dataPath
            , m_DefaultSoundFolder
            , m_ExtendSoundFolder);

        Debug.LogFormat("Copy SoundFile : {0}, {1}", resourcepath, streamassertpath);
        //FileUtil.CopyFileOrDirectory(resourcepath, streamassertpath);
        Editor_CopyAllFileNFolder(resourcepath, streamassertpath, true);
#endif

    }

    private void Editor_CopyAllFileNFolder(string p_resourcepath, string p_destpath, bool p_copysubdir)
    {

#if UNITY_EDITOR
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

    [ContextMenu("[EX사운드파일폴더정리 -> Wav파일만있는지 확인하기 ]")]
    private void Editor_AdjustResourceFolderDatas()
    {
        string filepath = string.Format("{0}"
            , m_ExtendSoundFolder);

        Debug.LogFormat("EX Shot Sound : {0}", m_ExtendSoundFolder);
        //BeepAudioClipArr = Resources.LoadAll<AudioClip>(m_ExtendSoundFolder);

        string filefolderpath = m_DefaultSoundFolder + m_ExtendSoundFolder;
        Editor_AdjustSoundFile(filefolderpath);
        Editor_CopyAdjustSoundDatas();
        Debug.LogFormat("Shot Sound Load 2 : {0}", BeepAudioClipList.Count);
    }

    
    protected void Editor_AdjustSoundFile(string p_assetfile)
    {

#if UNITY_EDITOR
        BeepAudioClipList.Clear();

        // Application.dataPath => projectfullpath/assets
        // System.IO.Path.GetFullPath(".") => projectfullpath

        string projectpath = Application.dataPath;// System.IO.Path.GetFullPath(".");
        string SoundFolder = p_assetfile;
        DirectoryInfo directoryinfo = new DirectoryInfo(projectpath + "/" + SoundFolder);
        FileInfo[] fiArr = directoryinfo.GetFiles();

        int count = fiArr.Length;
        AudioClip soundclip = null;

        int childcount = 0;
        string fileassetpath = "";
        string fileextend = "." + m_DefaultFileExtend;
        for (int i = 0; i < count; ++i)
        {

            if (fiArr[i].Extension.ToLower() == fileextend)
            {
                fileassetpath = "Assets/" + SoundFolder + fiArr[i].Name;
                soundclip = AssetDatabase.LoadAssetAtPath<AudioClip>(fileassetpath);

                if (soundclip == null)
                {
                    Debug.LogErrorFormat("Sound Null : {0}", fileassetpath);
                    continue;
                }

                BeepAudioClipList.Add(soundclip.name);
            }
        }

#endif

        Debug.LogFormat("File Adjust Clip : {0}", BeepAudioClipList.Count);

    }



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
            int soundId = EX_AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "playSound", new object[] { soundId } );
		}

		public static void playSound( int soundId, float volume ) {
			soundObj.Call( "playSound", new object[] { soundId, volume } );
		}

        public static void playSound( string p_soundname, float volume ) 
        {
            int soundId = EX_AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "playSound", new object[] { soundId, volume } );
		}
		
		public static void playSound( int soundId, float leftVolume, float rightVolume, int priority, int loop, float rate  ) {
			soundObj.Call( "playSound", new object[] { soundId, leftVolume, rightVolume, priority, loop, rate } );
		}
		

        public static void playSound( string p_soundname, float leftVolume, float rightVolume, int priority, int loop, float rate  ) 
        {
            int soundId = EX_AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "playSound", new object[] { soundId, leftVolume, rightVolume, priority, loop, rate } );
		}

		public static int loadSound( string soundName ) 
        {
            string fullname = string.Format("{0}{1}.{2}"
                , EX_AudioCenter.instance.m_FolderPath
                , soundName
                , EX_AudioCenter.instance.m_DefaultFileExtend);
            int soundID = soundObj.Call<int>( "loadSound", new object[] { fullname } );
            EX_AudioCenter.instance.m_SyncStringNInt.Add(soundName, soundID);
			return soundID;
		}
		
        public static void unloadSound( string p_soundname ) 
        {
            int soundId = EX_AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "unloadSound", new object[] { soundId } );

            EX_AudioCenter.instance.m_SyncStringNInt.Remove(p_soundname);
		}

		public static void unloadSound( int soundId ) {
			soundObj.Call( "unloadSound", new object[] { soundId } );
		}
#else
    private Dictionary<int, AudioClip> audioDic = new Dictionary<int, AudioClip>();


    public static void playSound(int soundId)
    {
        EX_AudioCenter.instance.audioSource.PlayOneShot(EX_AudioCenter.instance.audioDic[soundId]);
    }

    public static void playSound(string p_soundname)
    {
        int soundId = EX_AudioCenter.instance.m_SyncStringNInt[p_soundname];
        EX_AudioCenter.instance.audioSource.PlayOneShot(EX_AudioCenter.instance.audioDic[soundId]);
    }

    public static void playSound(int soundId, float volume)
    {
        EX_AudioCenter.instance.audioSource.PlayOneShot(EX_AudioCenter.instance.audioDic[soundId], volume);
    }

    public static void playSound(string p_soundname, float volume)
    {
        int soundId = EX_AudioCenter.instance.m_SyncStringNInt[p_soundname];
        EX_AudioCenter.instance.audioSource.PlayOneShot(EX_AudioCenter.instance.audioDic[soundId], volume);
    }

    public static void playSound(int soundId, float leftVolume, float rightVolume, int priority, int loop, float rate)
    {
        //float panRatio = EX_AudioCenter.instance.audioSource.panStereo;
        //rightVolume = Mathf.Clamp(rightVolume, 0, 1);
        //leftVolume = Mathf.Clamp(leftVolume, 0, 1);
        //EX_AudioCenter.instance.audioSource.panStereo = Mathf.Clamp(rightVolume, 0, 1) - Mathf.Clamp(leftVolume, 0, 1);
        float volume = (leftVolume + rightVolume) / 2;
        EX_AudioCenter.instance.audioSource.PlayOneShot(EX_AudioCenter.instance.audioDic[soundId], volume);
        //EX_AudioCenter.instance.audioSource.panStereo = panRatio;
    }


    public static void playSound(string p_soundname, float leftVolume, float rightVolume, int priority, int loop, float rate)
    {
        int soundId = EX_AudioCenter.instance.m_SyncStringNInt[p_soundname];
        float volume = (leftVolume + rightVolume) / 2;
        EX_AudioCenter.instance.audioSource.PlayOneShot(EX_AudioCenter.instance.audioDic[soundId], volume);
    }

//         public static void SetSoundFolder(string p_folderurl)
//         {
//             EX_AudioCenter.instance.m_DefaultSoundFolder = p_folderurl;
//         }

    public static AudioClip Editor_GetAudioClip(string soundName)
    {
#if !UNITY_ANDROID || UNITY_EDITOR
        string filefolderpath = EX_AudioCenter.instance.m_DefaultSoundFolder + EX_AudioCenter.instance.m_ExtendSoundFolder;
        filefolderpath = "Assets/" + filefolderpath + soundName + "." + EX_AudioCenter.instance.m_DefaultFileExtend;
        //filefolderpath = StringManager.SumString("Assets/", filefolderpath, soundName, ".", EX_AudioCenter.instance.m_DefaultFileExtend);
        AudioClip audioClip = AssetDatabase.LoadAssetAtPath<AudioClip>(filefolderpath);
        return audioClip;
#else
        return null;
#endif
    }

    public static int loadSound(string soundName)
    {
        int soundID = soundName.GetHashCode();
        string fullname = string.Format("{0}{1}", EX_AudioCenter.instance.m_ExtendSoundFolder, soundName);

        AudioClip audioClip = Editor_GetAudioClip(soundName);


        //AudioClip audioClip = Resources.Load<AudioClip>("Sounds/" + soundName);
        //AudioClip audioClip = Resources.Load<AudioClip>(fullname);
        //EX_AudioCenter.instance.audioDic[soundID] = audioClip;
        //EX_AudioCenter.instance.m_SyncStringNInt[soundName] = soundID;



        if (audioClip == null)
        {
            Debug.LogErrorFormat(" Audio Sound NullData : {0}", fullname);
        }

        
        EX_AudioCenter.instance.audioDic.Add(soundID, audioClip);
        EX_AudioCenter.instance.m_SyncStringNInt.Add(soundName, soundID);
        
        string tempfullname = string.Format("{0}{1}.{2}"
            , EX_AudioCenter.instance.m_FolderPath
            , soundName
            , EX_AudioCenter.instance.m_DefaultFileExtend);

        Debug.Log("Test FullName : " + tempfullname);

        return soundID;
    }

    public static void unloadSound(int soundId)
    {
        var audioClip = EX_AudioCenter.instance.audioDic[soundId];
        Resources.UnloadAsset(audioClip);
        EX_AudioCenter.instance.audioDic.Remove(soundId);
    }

    public static void unloadSound(string p_soundname)
    {
        int soundId = EX_AudioCenter.instance.m_SyncStringNInt[p_soundname];
        var audioClip = EX_AudioCenter.instance.audioDic[soundId];
        Resources.UnloadAsset(audioClip);
        EX_AudioCenter.instance.audioDic.Remove(soundId);
        EX_AudioCenter.instance.m_SyncStringNInt.Remove(p_soundname);
    }

#endif

    private void Awake()
    {
        if (instance == null || instance == this)
        {
            instance = this;
        }
        else
        {
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
        int count = BeepAudioClipList.Count;
        for (int i = 0; i < count; i++)
        {
            Debug.Log(" Init Loader Audio clip name : " + BeepAudioClipList[i] );
            EX_AudioCenter.loadSound(BeepAudioClipList[i] );
        }
    }


    // 전처리기에서 걸러짐
    [Header("[DefaultBeepSound]")]
    //public AudioClip[] BeepAudioClipArr;

    [AudioClipAttribute]
    public List<string> BeepAudioClipList;

    

    public void OnPlaySound(AudioClip p_audioclip)
    {
        if (m_SyncStringNInt.ContainsKey(p_audioclip.name))
        {
            EX_AudioCenter.playSound(p_audioclip.name);
        }
        else
        {
            Debug.LogError("Miss Matching AudioClip : ");
        }

    }

    public static void PlaySound(AudioClip p_audioclip)
    {
        if (EX_AudioCenter.instance.m_SyncStringNInt.ContainsKey(p_audioclip.name))
        {
            EX_AudioCenter.playSound(p_audioclip.name);
        }
        else
        {
            Debug.LogError(" Static PlaySound Miss Matching AudioClip : ");
        }
    }

    public static void PlaySound(string p_audioclip)
    {
        if (EX_AudioCenter.instance.m_SyncStringNInt.ContainsKey(p_audioclip))
        {
            EX_AudioCenter.playSound(p_audioclip);
        }
        else
        {
            Debug.LogError(" Static PlaySound Miss Matching AudioClip Str : ");
        }
    }


    void Start ()
    {
		
	}

    void Update()
    {

    }
}
