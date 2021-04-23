using System.Collections;
using System.Collections.Generic;
using System.Linq;
using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class Game_Management : MonoBehaviour
{
    public List<GameObject> Original_Skill_List;
    public List<GameObject> Current_Skill_List;
    
    public RectTransform[] Random_Skills = new RectTransform[3];
    public RectTransform[] Empty_Skill_Position = new RectTransform[3];

    public Button_Management btn_manager;

    
    // Start is called before the first frame update
    void Awake()
    {
        btn_manager = GameObject.Find("Button_Manager").GetComponent<Button_Management>();
        Original_Skill_List = GameObject.FindGameObjectsWithTag("Skill").ToList();

        Current_Skill_List = new List<GameObject>(Original_Skill_List);
    }
    void Start()
    {
        StartCoroutine(Random_Set_Skill());
    }

    IEnumerator Random_Set_Skill()
    {
        while(true)
        {
            if(btn_manager.is_Choice && !btn_manager.is_Created)
            {
                btn_manager.is_Created = true;
                btn_manager.is_Choice = false;
                for (int i = 0; i < Random_Skills.Length; i++)
                {
                    int idx = Random.Range(0, Current_Skill_List.Count);
                    Random_Skills[i] = Current_Skill_List[idx].GetComponent<RectTransform>();
                    Current_Skill_List.RemoveAt(idx);
                }

                Current_Skill_List = new List<GameObject>(Original_Skill_List);

                for (int i = 0; i < Random_Skills.Length; i++)
                {
                    RectTransform temp = GameObject.Instantiate(Random_Skills[i]) as RectTransform;
                    temp.gameObject.SetActive(true);
                    temp.transform.SetParent(Empty_Skill_Position[i]);
                    temp.name = Random_Skills[i].name;
                    temp.anchoredPosition = Vector2.zero;

                    temp.SetSizeWithCurrentAnchors(RectTransform.Axis.Horizontal, 100);
                    temp.SetSizeWithCurrentAnchors(RectTransform.Axis.Vertical, 100);

                    temp.localPosition = Vector2.zero;
                    btn_manager.AddSkill(temp);
                }
                
            }
            yield return new WaitForSeconds(2f);
        }
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
