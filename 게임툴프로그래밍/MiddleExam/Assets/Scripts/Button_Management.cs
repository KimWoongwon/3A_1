using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Button_Management : MonoBehaviour
{
    public GameObject Preset_UI;
    public GameObject Exit_Button;
    public GameObject Show_Button;

    public RectTransform ScrollView;
    public GameObject Left_Wall_Button;
    public Sprite right_sprite;
    public Sprite left_sprite;
    bool isbtnClicked = false;

    public GameObject Skill_Select_Button_01;
    public GameObject Skill_Select_Button_02;
    public GameObject Skill_Select_Button_03;

    Scroll_Management Scroll_Management = null;
    public List<RectTransform> Shown_Skills = new List<RectTransform>(3);
    public bool is_Choice = true;
    public bool is_Created = false;

    const float movevalue = 200f;
    const int countvalue = 40;

    #region Button_Event_Function
    public void OnButtonClicked()
    {
        isbtnClicked = !isbtnClicked;
        if (isbtnClicked)
            StartCoroutine(move_right_scroll());
        else
            StartCoroutine(move_left_scroll());
    }
    public void ShowPreset()
    {
        if (isbtnClicked)
            ScrollView.gameObject.SetActive(false);

        Preset_UI.SetActive(true);
        Exit_Button.SetActive(true);

        Show_Button.SetActive(false);
        return;
    }
    public void HidePreset()
    {
        if (isbtnClicked)
            ScrollView.gameObject.SetActive(true);

        Preset_UI.SetActive(false);
        Exit_Button.SetActive(false);

        Show_Button.SetActive(true);
        return;
    }

    public void On_Skill_Select_01()
    {
        if (!is_Created)
            return;
        Debug.Log("Select Skill 01");
        Scroll_Management.InsertSkill(Shown_Skills[0]);
    }
    public void On_Skill_Select_02()
    {
        if (!is_Created)
            return;
        Debug.Log("Select Skill 02");
        Scroll_Management.InsertSkill(Shown_Skills[1]);
    }
    public void On_Skill_Select_03()
    {
        if (!is_Created)
            return;
        Debug.Log("Select Skill 03");
        Scroll_Management.InsertSkill(Shown_Skills[2]);
    }

    public void OnChoose()
    {
        if (!is_Created)
            return;
        Debug.Log("On_Choose");
        is_Choice = true;

        for (int i = Shown_Skills.Count - 1; i >= 0; i--)
        {
            Destroy(Shown_Skills[i].gameObject);
            Shown_Skills.RemoveAt(i);
        }
        is_Created = false;
    }

    public void OnResetCharacterSkill()
    {
        Scroll_Management.ResetSkill();
    }

    #endregion

    #region Coroutine_Function
    IEnumerator move_right_scroll()
    {
        int count = 0;
        while (true)
        {
            Vector3 temp = ScrollView.transform.position;
            temp.x += movevalue / (float)countvalue;
            ScrollView.transform.position = temp;
            ++count;
            if (count >= countvalue)
                break;
            yield return new WaitForSeconds(0.01f);
        }

        Left_Wall_Button.GetComponent<Image>().sprite = left_sprite;
        Debug.Log("right_Scroll_End");
        yield return 0;
    }

    IEnumerator move_left_scroll()
    {
        int count = 0;
        while (true)
        {
            Vector3 temp = ScrollView.transform.position;
            temp.x -= movevalue / (float)countvalue;
            ScrollView.transform.position = temp;
            ++count;
            if (count >= countvalue)
                break;
            yield return new WaitForSeconds(0.01f);
        }

        Left_Wall_Button.GetComponent<Image>().sprite = right_sprite;
        Debug.Log("left_Scroll_End");
        yield return 0;
    }
    #endregion

    public void AddSkill(RectTransform _skill)
    {
        Shown_Skills.Add(_skill);
    }

    // Start is called before the first frame update
    void Start()
    {
        Scroll_Management = GameObject.Find("Scroll_Manager").GetComponent<Scroll_Management>();
        Vector3 temp = ScrollView.transform.position;
        temp.x -= 200f;
        ScrollView.transform.position = temp;

        Preset_UI.SetActive(false);
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
