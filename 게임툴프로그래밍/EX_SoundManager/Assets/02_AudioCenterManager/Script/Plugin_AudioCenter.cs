using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

#if UNITY_EDITOR
using UnityEditor;
#endif


namespace Du3Project
{
	public class Plugin_AudioCenter : MonoBehaviour
	{

        #region 싱글톤

        static Plugin_AudioCenter m_Instance = null;

        public static Plugin_AudioCenter GetI
        {
            get
            {
                if (m_Instance == null)
                {
                    m_Instance = GameObject.FindObjectOfType<Plugin_AudioCenter>();
                    if (m_Instance == null)
                    {
                        Debug.LogErrorFormat("Plugin_AudioCenter 가 씬안에 없습니다. 확인하세요");
                    }
                }

                return m_Instance;
            }
        }
        #endregion


        [SerializeField]
        protected AudioSource audioSource;

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



        [SerializeField]
        protected Dictionary<string, int> m_SyncStringNInt = new Dictionary<string, int>();
        private Dictionary<int, AudioClip> audioDic = new Dictionary<int, AudioClip>();


        private void Editor_CopyAdjustSoundDatas()
        {
#if UNITY_EDITOR
            // 기존 사운드 파일 정보 위치
            string resourcepath = string.Format("{0}/{1}{2}"
                , Application.dataPath
                , m_DefaultSoundFolder
                , m_ExtendSoundFolder);

            // StreamingAssets 폴더 정보
            string streamassertpath = string.Format("{0}/StreamingAssets/{1}{2}"
                , Application.dataPath
                , m_DefaultSoundFolder
                , m_ExtendSoundFolder);

            Debug.LogFormat("Copy SoundFile : {0}, {1}", resourcepath, streamassertpath);
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


//#if UNITY_EDITOR
//        [Header("[DefaultBeepSound]")]
//        public AudioClip[] BeepAudioClipArr;

//        [ContextMenu("[사운드파일폴더정리 -> Wav파일만있는지 확인하기 ]")]
//        private void Editor_AdjustResourceFolderDatas()
//        {
//            string filepath = string.Format("{0}"
//                , m_ExtendSoundFolder);

//            Debug.LogFormat("Shot Sound : {0}", m_ExtendSoundFolder);
//            BeepAudioClipArr = Resources.LoadAll<AudioClip>(m_ExtendSoundFolder);

//            Editor_CopyAdjustSoundDatas();
//            Debug.LogFormat("Shot Sound Load 2 : {0}", BeepAudioClipArr.Length);
//        }
//#endif


        //public void OnPlaySound(AudioClip p_audioclip)
        //{
        //    if( Plugin_AudioCenter.GetI.m_SyncStringNInt.ContainsKey(p_audioclip.name) )
        //    {

        //    }
        //    else
        //    {
        //        Debug.LogErrorFormat("사운드 값이 없음 에러임 : {0}", p_audioclip.name);
        //    }

        //}

        //public void OnPlaySound(string p_audioclipstr)
        //{
        //    if (Plugin_AudioCenter.GetI.m_SyncStringNInt.ContainsKey(p_audioclipstr))
        //    {

        //    }
        //    else
        //    {
        //        Debug.LogErrorFormat("사운드 값이 없음 에러임2 : {0}", p_audioclipstr);
        //    }
        //}



        public void InitDefaultSoundLoader(Dictionary<E_SSOUND_NAME, AudioClip> p_sfxdict)
        {

#if UNITY_EDITOR || UNITY_WINDOW

            foreach (var item in p_sfxdict)
            {
                Debug.Log(" Init Loader Audio clip name : " + item.Value.name);
                Plugin_AudioCenter.loadSound(item.Value.name);
            }

            //int count = BeepAudioClipArr.Length;
            //for (int i = 0; i < count; i++)
            //{
            //    Debug.Log(" Init Loader Audio clip name : " + BeepAudioClipArr[i].name);
            //    Plugin_AudioCenter.loadSound(BeepAudioClipArr[i].name);

            //}
#endif

        }


        public static int loadSound(string soundName)
        {
#if UNITY_ANDROID && !UNITY_EDITOR
            string fullname = string.Format("{0}{1}.{2}"
                , AudioCenter.instance.m_FolderPath
                , soundName
                , AudioCenter.instance.m_DefaultFileExtend);
            int soundID = soundObj.Call<int>( "loadSound", new object[] { fullname } );
            AudioCenter.instance.m_SyncStringNInt.Add(soundName, soundID);
			return soundID;
#else
            int soundID = soundName.GetHashCode();
            AudioClip clip = SoundManagerN.GetI.GetDictAudioClip(soundName);

            Plugin_AudioCenter.m_Instance.audioDic.Add(soundID, clip);
            Plugin_AudioCenter.m_Instance.m_SyncStringNInt.Add(soundName, soundID);

            return soundID;


            //int soundID = soundName.GetHashCode();
            //string fullname = string.Format("{0}{1}", Plugin_AudioCenter.m_Instance.m_ExtendSoundFolder, soundName);

            //AudioClip audioClip = null;

            //if (audioClip == null)
            //{
            //    Debug.LogErrorFormat(" Audio Sound NullData : {0}", fullname);
            //}

            //Plugin_AudioCenter.m_Instance.audioDic.Add(soundID, audioClip);
            //Plugin_AudioCenter.m_Instance.m_SyncStringNInt.Add(soundName, soundID);

            //string tempfullname = string.Format("{0}{1}.{2}"
            //    , Plugin_AudioCenter.m_Instance.m_FolderPath
            //    , soundName
            //    , Plugin_AudioCenter.m_Instance.m_DefaultFileExtend);

            //return soundID;

#endif
        }

        public static void unloadSound(int soundId)
        {
#if UNITY_ANDROID && !UNITY_EDITOR
            soundObj.Call( "unloadSound", new object[] { soundId } );
#else
            var audioClip = Plugin_AudioCenter.m_Instance.audioDic[soundId];
            if(audioClip != null)
            {
                Plugin_AudioCenter.m_Instance.m_SyncStringNInt.Remove(audioClip.name);
                Resources.UnloadAsset(audioClip);
            }

            Plugin_AudioCenter.m_Instance.audioDic.Remove(soundId);
#endif
        }

        public static void unloadSound(string p_soundname)
        {
#if UNITY_ANDROID && !UNITY_EDITOR
            int soundId = Plugin_AudioCenter.m_Instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "unloadSound", new object[] { soundId } );
            Plugin_AudioCenter.m_Instance.m_SyncStringNInt.Remove(p_soundname);
#else
            int soundId = Plugin_AudioCenter.m_Instance.m_SyncStringNInt[p_soundname];
            var audioClip = Plugin_AudioCenter.m_Instance.audioDic[soundId];
            Resources.UnloadAsset(audioClip);
            Plugin_AudioCenter.m_Instance.audioDic.Remove(soundId);
            Plugin_AudioCenter.m_Instance.m_SyncStringNInt.Remove(p_soundname);
#endif
        }



        public static void playSound(int soundId)
        {
#if UNITY_ANDROID && !UNITY_EDITOR
            soundObj.Call( "playSound", new object[] { soundId } );
#else
            Plugin_AudioCenter.m_Instance.audioSource.PlayOneShot(
                Plugin_AudioCenter.m_Instance.audioDic[soundId]);
#endif

        }

        public static void playSound(string p_soundname)
        {
#if UNITY_ANDROID && !UNITY_EDITOR
            nt soundId = AudioCenter.instance.m_SyncStringNInt[p_soundname];
			soundObj.Call( "playSound", new object[] { soundId } );
#else
            int soundId = Plugin_AudioCenter.m_Instance.m_SyncStringNInt[p_soundname];
            Plugin_AudioCenter.m_Instance.audioSource.PlayOneShot(
                Plugin_AudioCenter.m_Instance.audioDic[soundId]);
#endif

        }

        public static void playSound(int soundId, float volume)
        {
#if UNITY_ANDROID && !UNITY_EDITOR
            soundObj.Call( "playSound", new object[] { soundId, volume } );
#else
            Plugin_AudioCenter.m_Instance.audioSource.PlayOneShot(
                Plugin_AudioCenter.m_Instance.audioDic[soundId], volume);
#endif

        }

        public static void playSound(string p_soundname, float volume)
        {
#if UNITY_ANDROID && !UNITY_EDITOR
            int soundId = Plugin_AudioCenter.m_Instance.m_SyncStringNInt[p_soundname];
            soundObj.Call("playSound", new object[] { soundId, volume });
#else
            int soundId = Plugin_AudioCenter.m_Instance.m_SyncStringNInt[p_soundname];
            Plugin_AudioCenter.m_Instance.audioSource.PlayOneShot(Plugin_AudioCenter.m_Instance.audioDic[soundId], volume);
#endif
        }


        public static AndroidJavaClass unityActivityClass;
        public static AndroidJavaObject activityObj;
        private static AndroidJavaObject soundObj;

        private void Awake()
        {
        }


        void Start()
		{
#if !UNITY_ANDROID || UNITY_EDITOR
            if (audioSource == null)
            {
                audioSource = gameObject.AddComponent<AudioSource>();

            }
#else


#endif
            // 자바에 있는 jar plugin 사용하기
            unityActivityClass = new AndroidJavaClass("com.unity3d.player.UnityPlayer");
            activityObj = unityActivityClass.GetStatic<AndroidJavaObject>("currentActivity");
            soundObj = new AndroidJavaObject("com.catsknead.androidsoundfix.AudioCenter"
                                , 5
                                , activityObj);


            //InitDefaultSoundLoader();
        }


		void Update()
		{
			
		}

	}
}