using System.Linq;
using UnityEngine;
using System.Collections;
using UnityEditor;
using System.IO;
using System.Collections.Generic;



//  이미 씬파일에서 리소스들을 들고 있기때문에 변화된점이 없음 
// 스트링으로 들고 있으면 좋지만 에디터 상에서 파일을 로드할수가 없음
// 
[CustomEditor(typeof(EX_AudioCenter))]
public class EX_AudioCenter_Editor : Editor
{

    EX_AudioCenter m_targetobj = null;
    void OnEnable()
    {
        m_targetobj = target as EX_AudioCenter;

    }
    public override void OnInspectorGUI()
    {
        base.OnInspectorGUI();
        
        GUILayout.Space(15f);
        return;

        if (GUILayout.Button("[Extendtion 파일 정리]"))
        {
            //m_targetobj.BeepAudioClipArr;

            string filefolderpath = m_targetobj.DefaultSoundFolder + m_targetobj.ExtendSoundFolder;
            Editor_AdjustSoundFile(filefolderpath);
            Editor_CopyAdjustSoundDatas();
        }
    }

    List<AudioClip> m_TempAudioClipList = new List<AudioClip>();
    protected void Editor_AdjustSoundFile(string p_assetfile)
    {
        m_TempAudioClipList.Clear();


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
        string fileextend = "." + m_targetobj.DefaultFileExtend;
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

                m_TempAudioClipList.Add(soundclip);
            }
        }


        m_targetobj.BeepAudioClipList.Clear();
        foreach (var item in m_TempAudioClipList)
        {
            m_targetobj.BeepAudioClipList.Add( item.name );
        }

        Debug.LogFormat("File Adjust Clip : {0}", m_TempAudioClipList.Count );
        
    }






    private void Editor_CopyAdjustSoundDatas()
    {
#if UNITY_EDITOR
        string resourcepath = string.Format("{0}/{1}{2}"
            , Application.dataPath
            , m_targetobj.DefaultSoundFolder
            , m_targetobj.ExtendSoundFolder);

        string streamassertpath = string.Format("{0}/StreamingAssets/{1}{2}"
            , Application.dataPath
            , ""//m_targetobj.DefaultSoundFolder
            , m_targetobj.ExtendSoundFolder);

        Debug.LogFormat("Copy SoundFile : {0}, {1}", resourcepath, streamassertpath);
        //FileUtil.CopyFileOrDirectory(resourcepath, streamassertpath);
        Editor_CopyAllFileNFolder(resourcepath, streamassertpath, true);
#endif

    }

    private void Editor_CopyAllFileNFolder(string p_resourcepath, string p_destpath, bool p_copysubdir)
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



	
}





