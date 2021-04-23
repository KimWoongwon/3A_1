#if UNITY_EDITOR
using UnityEditor;
#endif

using System.IO;
using UnityEngine;


#if UNITY_EDITOR

[CustomPropertyDrawer (typeof (AudioClipAttribute))]
public class AudioClipDrawer : PropertyDrawer {
	

	public override void OnGUI (Rect position, SerializedProperty property, GUIContent label) {
		
		if (property.propertyType == SerializedPropertyType.String) {
            EX_AudioCenter targetobj = property.serializedObject.targetObject as EX_AudioCenter;
            var audioobject = GetAudioClipObject(property.stringValue, targetobj);
			var audioclipdata = EditorGUI.ObjectField(position, label, audioobject, typeof(AudioClip), true);
			if (audioclipdata == null) {
				property.stringValue = "";
			} else if (audioclipdata.name != property.stringValue) {
				var sceneObj = GetAudioClipObject(audioclipdata.name, targetobj);
				if (sceneObj == null) {
					Debug.LogError("The AudioClip " + audioclipdata.name + " cannot be used. AudioClip Find");
				} else {
					property.stringValue = audioclipdata.name;
				}
			}
		}
		else
			EditorGUI.LabelField (position, label.text, "Use [AudioClip] with strings.");
	}

	protected AudioClip GetAudioClipObject(string audioclipobjectname, EX_AudioCenter p_audiocenter ) {
		if (string.IsNullOrEmpty(audioclipobjectname)) {
			return null;
		}

        
        string projectpath = Application.dataPath;// System.IO.Path.GetFullPath(".");
        string SoundFolder = p_audiocenter.DefaultSoundFolder + p_audiocenter.ExtendSoundFolder;
        DirectoryInfo directoryinfo = new DirectoryInfo(projectpath + "/" + SoundFolder);

        audioclipobjectname = "Assets/" + SoundFolder + audioclipobjectname + "." + p_audiocenter.DefaultFileExtend;

        AudioClip outclip = AssetDatabase.LoadAssetAtPath(audioclipobjectname, typeof(AudioClip)) as AudioClip;
        if (outclip == null)
        {
            Debug.LogError("AudioClip [" + audioclipobjectname + "] AudioClip : ");
        }
        return outclip;


//         foreach (var editorScene in EditorBuildSettings.scenes) {
// 			if (editorScene.path.IndexOf(audioclipobjectname) != -1) {
// 				return AssetDatabase.LoadAssetAtPath(editorScene.path, typeof(AudioClip)) as AudioClip;
// 			}
// 		}
// 		Debug.LogWarning("Scene [" + audioclipobjectname + "] cannot be used. Add this scene to the 'Scenes in the Build' in build settings.");
// 		return null;
	}
}

#endif