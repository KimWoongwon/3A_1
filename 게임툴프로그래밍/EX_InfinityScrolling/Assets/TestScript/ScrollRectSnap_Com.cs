using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;


// https://www.youtube.com/watch?v=ETqdTIUSf3M
// Unity 5 UI Tutorial - Scroll Rect Snap to Element : Part 04

public class ScrollRectSnap_Com : MonoBehaviour 
{

    public RectTransform panel; //ScrollPanel
    public Button[] bttn; 
    public RectTransform center;
    public int startButton = 1;


    public float[] distacnce;
    public float[] distReposition;

    private bool draging = false;
    private int bttnDistance;
    private int minButtonNum;
    private int bttnLength;
    private bool messageSend = false;
    private float LerpSpeed = 5f;
    private bool targetNearestButton = true;



	void Start () 
    {
        bttnLength = bttn.Length;
        distacnce = new float[bttnLength];
        distReposition = new float[bttnLength];

        bttnDistance = (int)Mathf.Abs(bttn[1].GetComponent<RectTransform>().anchoredPosition.x 
            - bttn[0].GetComponent<RectTransform>().anchoredPosition.x);

        panel.anchoredPosition = new Vector2( (startButton - 1) * -300f, 0f );
	}
	

	void Update () 
    {
		for (int i=0; i< bttn.Length; ++i)
		{
            distReposition[i] = center.GetComponent<RectTransform>().position.x - bttn[i].GetComponent<RectTransform>().position.x;
            distacnce[i] = Mathf.Abs( distacnce[i] );


            if ( distReposition[i] > 1200 )
            {
                float curX = bttn[i].GetComponent<RectTransform>().anchoredPosition.x;
                float curY = bttn[i].GetComponent<RectTransform>().anchoredPosition.y;

                Vector2 newAnchoredPos = new Vector2(curX + (bttnLength * bttnDistance), curY );
                bttn[i].GetComponent<RectTransform>().anchoredPosition = newAnchoredPos;
            }

            if ( distReposition[i] < -1200 )
            {
                float curX = bttn[i].GetComponent<RectTransform>().anchoredPosition.x;
                float curY = bttn[i].GetComponent<RectTransform>().anchoredPosition.y;

                Vector2 newAnchoredPos = new Vector2(curX - (bttnLength * bttnDistance), curY);
                bttn[i].GetComponent<RectTransform>().anchoredPosition = newAnchoredPos;
            }
		}


        if (targetNearestButton)
        {
            float minDistance = Mathf.Min(distacnce);

            for (int i=0; i< bttn.Length; ++i)
            {
            	if ( minDistance == distacnce[i])
            	{
                    minButtonNum = i;
            	}
            }
        }

        if (!draging)
        {
            LerpToBttn( -bttn[minButtonNum].GetComponent<RectTransform>().anchoredPosition.x );
        }


	}



    void LerpToBttn(float position)
    {
        float newX = Mathf.Lerp(panel.anchoredPosition.x, position, Time.deltaTime * LerpSpeed);

        if (Mathf.Abs(position - newX) < 3f)
            newX = position;


        if ( Mathf.Abs(newX) >= Mathf.Abs(position) - 1f
            && Mathf.Abs(newX) <= Mathf.Abs(position) + 1f
            && !messageSend )
        {
            messageSend = true;
            SendMessageFromButton(minButtonNum);
        }

        Vector2 newPosition = new Vector2(newX, panel.anchoredPosition.y );
        panel.anchoredPosition = newPosition;


    }

    void SendMessageFromButton(int bttnIndex)
    {
        if (bttnIndex - 1 == 3)
            Debug.LogFormat("Message Send");

    }


    public void StartDrag()
    {
        messageSend = false;
        LerpSpeed = 5f;
        draging = true;

        targetNearestButton = true;
    }

    public void EndDrag()
    {
        draging = false;
    }

    public void GoToButton(int buttonIndex)
    {
        targetNearestButton = false;
        minButtonNum = buttonIndex - 1;

    }




}
