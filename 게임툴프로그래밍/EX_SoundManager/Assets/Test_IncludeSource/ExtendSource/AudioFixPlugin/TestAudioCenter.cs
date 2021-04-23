using UnityEngine;
using System.Collections;



public class TestAudioCenter : MonoBehaviour 
{

    public AudioClip[] BeepAudioClipArr;


    public void OnPlaySound( AudioClip p_audioclip )
    {
        AudioCenter.playSound(p_audioclip.name);
    }


    void Awake()
    {
        //AudioCenter.SetSoundFolder()
        
        
    }

	IEnumerator Start () 
    {
        yield return null;

        foreach (var item in BeepAudioClipArr)
        {
            Debug.Log(" clip name : " + item.name);
            AudioCenter.loadSound(item.name);
        }
	}
	
	
}
