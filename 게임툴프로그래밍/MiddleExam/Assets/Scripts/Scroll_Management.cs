using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Scroll_Management : MonoBehaviour
{
    public RectTransform Skill_Parent;
    public RectTransform Contant;
    public List<RectTransform> Character_Skill_List;

    public Vector2 Original_Contant_Size;

    void Start()
    {
        Character_Skill_List = new List<RectTransform>();
        Original_Contant_Size = Contant.sizeDelta;
    }

    public void InsertSkill(RectTransform _rt)
    {
        var temp = GameObject.Instantiate(_rt) as RectTransform;
        temp.SetParent(Skill_Parent);
        temp.gameObject.SetActive(true);

        Vector2 size = Contant.sizeDelta;
        size.y += temp.sizeDelta.y + 50f;
        Contant.sizeDelta = size;

        Character_Skill_List.Add(temp);
    }

    public void ResetSkill()
    {
        for (int i = Character_Skill_List.Count - 1; i >= 0; i--)
        {
            Destroy(Character_Skill_List[i].gameObject);
            Character_Skill_List.RemoveAt(i);
        }
        Contant.sizeDelta = Original_Contant_Size;
    }

    void Update()
    {
        
    }
}
