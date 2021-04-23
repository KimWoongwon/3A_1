using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

namespace Du3Project
{
	public class UIItemElement : MonoBehaviour
	{
        [Header("[자료들]")]
        public Text ItemLabel = null;
        public Text ItemScore = null;
        public Image ItemIcon = null;

        [Header("[데이터용Index]")]
        public int ItemIndex = -1;


        public void UpdateItem(int p_index)
        {
            ItemIndex = p_index;

            ItemScore.text = ItemIndex.ToString();
        }

        void Start()
		{
			
		}

		//void Update()
		//{
			
		//}
	}
}