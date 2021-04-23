using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Du3Project
{
    [RequireComponent(typeof(Button))]
    public class Sample_UIClick : MonoBehaviour
    {
        public E_SUISoundType UISoundType = E_SUISoundType.Max;

        void InitUIClick()
        {
            GetComponent<Button>().onClick.AddListener(ClickSound);
        }

        void ClickSound()
        {
            Sample_SoundManager.GetI.ShotSoundSFX(UISoundType);
        }

        void Start()
        {
            InitUIClick();

        }


        void Update()
        {

        }
    }
}