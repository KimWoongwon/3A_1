#if UNITY_EDITOR
using UnityEditor;
#endif
using UnityEngine;

#if UNITY_EDITOR

[CustomPropertyDrawer(typeof(EX_AudioClipAttribute))]
public class EX_AudioClipDrawer : PropertyDrawer
{
    public override void OnGUI(Rect position, SerializedProperty property, GUIContent label)
    {
        base.OnGUI(position, property, label);
        

        if (property.propertyType == SerializedPropertyType.String)
        {
            var sceneObject = GetAudioClipObject(property.stringValue);
            var scene = EditorGUI.ObjectField(position, label, sceneObject, typeof(AudioClip), true);
            if (scene == null)
            {
                property.stringValue = "";
            }
            else if (scene.name != property.stringValue)
            {
                var sceneObj = GetAudioClipObject(scene.name);
                if (sceneObj == null)
                {
                    Debug.LogWarning("The scene " + scene.name + " cannot be used. To use this scene add it to the build settings for the project");
                }
                else
                {
                    property.stringValue = scene.name;
                }
            }
        }
        else
            EditorGUI.LabelField(position, label.text, "Use [Scene] with strings.");
    }
    protected AudioClip GetAudioClipObject(string sceneObjectName)
    {
        if (string.IsNullOrEmpty(sceneObjectName))
        {
            return null;
        }

        foreach (var editorScene in EditorBuildSettings.scenes)
        {
            if (editorScene.path.IndexOf(sceneObjectName) != -1)
            {
                return AssetDatabase.LoadAssetAtPath(editorScene.path, typeof(AudioClip)) as AudioClip;
            }
        }
        Debug.LogWarning("Scene [" + sceneObjectName + "] cannot be used. Add this scene to the 'Scenes in the Build' in build settings.");
        return null;
    }


}

#endif