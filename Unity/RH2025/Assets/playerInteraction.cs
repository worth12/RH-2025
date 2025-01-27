using Meta.XR.MRUtilityKit;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class playerInteraction : MonoBehaviour
{

    public OVRCameraRig _cameraRig;
    private Transform RightcontrollerPoint;
    private GameObject _debugNormal;

    public mqttReceiver _mqttReceiver;

    float distanceOffset = 1.0f;
    // Start is called before the first frame update
    void Start()
    {
        if (_cameraRig == null)
        {
            _cameraRig = GameObject.FindObjectOfType<OVRCameraRig>();
        }
        //MRUK.Instance?.RegisterSceneLoadedCallback(OnSceneLoaded);
        CreateDebugPrimitives();
    }

    private void OnSceneLoaded()
    {
        //CreateDebugPrimitives();
    }

    // Update is called once per frame
    void Update()
    {



    }

    void CreateDebugPrimitives()
    {
        _debugNormal = GameObject.CreatePrimitive(PrimitiveType.Cylinder);
        _debugNormal.name = "SceneDebugger_Normal";
        _debugNormal.GetComponent<Renderer>().material.color = Color.green;
        _debugNormal.transform.localScale = new Vector3(0.5f, 0.01f, 0.5f);
        _debugNormal.GetComponent<Collider>().enabled = false;
        _debugNormal.SetActive(false);
    }


    private Ray GetControllerRay()
    {
        Vector3 rayOrigin;
        Vector3 rayDirection;
        if (OVRInput.activeControllerType == OVRInput.Controller.Touch
            || OVRInput.activeControllerType == OVRInput.Controller.RTouch)
        {
            rayOrigin = _cameraRig.rightHandOnControllerAnchor.position;
            rayDirection = _cameraRig.rightHandOnControllerAnchor.forward;
        }
        else if (OVRInput.activeControllerType == OVRInput.Controller.LTouch)
        {
            rayOrigin = _cameraRig.leftHandOnControllerAnchor.position;
            rayDirection = _cameraRig.leftHandOnControllerAnchor.forward;
        }
        else // hands
        {
            var rightHand = _cameraRig.rightHandAnchor.GetComponentInChildren<OVRHand>();
            // can be null if running in Editor with Meta Linq app and the headset is put off
            if (rightHand != null)
            {
                rayOrigin = rightHand.PointerPose.position;
                rayDirection = rightHand.PointerPose.forward;
            }
            else
            {
                rayOrigin = _cameraRig.centerEyeAnchor.position;
                rayDirection = _cameraRig.centerEyeAnchor.forward;
            }
        }

        return new Ray(rayOrigin, rayDirection);
    }
    private void ShowHitNormal(Vector3 position, Vector3 normal)
    {
        if (_debugNormal != null && position != Vector3.zero && normal != Vector3.zero)
        {
            _debugNormal.SetActive(true);
            _debugNormal.transform.rotation = Quaternion.FromToRotation(-Vector3.up, normal);
            _debugNormal.transform.position =
                position + -_debugNormal.transform.up * _debugNormal.transform.localScale.y;
        }
        else
        {
            _debugNormal.SetActive(false);
        }
    }




    //on trigger pull, point OVRraycast from controller
    public void OnTriggerPull()
    {
        var ray = GetControllerRay();
        var hit = new RaycastHit();
        MRUKAnchor anchorHit = null;
        MRUK.Instance?.GetCurrentRoom()?.Raycast(ray, Mathf.Infinity, out hit, out anchorHit);
        //ShowHitNormal(hit.point, hit.normal);
        if (anchorHit != null)
        {
            print("Hit anchor: " + anchorHit.name + "Point: " + hit.point + "Norm: " + hit.normal);
            //if floor is hit, get distance from point to _cameraRig
            if (anchorHit.name.Contains("FLOOR"))
            {
                //
                var distance = Vector3.Distance(hit.point, _cameraRig.centerEyeAnchor.position)- distanceOffset;
                //print("Distance to floor: " + distance);
                //get angle from user 0 ,0,0 to hit point
                var angle = Vector3.Angle(Vector3.forward, hit.point - _cameraRig.centerEyeAnchor.position);
                print("Angle to floor: " + angle);
                //_mqttReceiver.sendDistance(distance, angle);
                ShowHitNormal(hit.point, hit.normal);

            }
            else if (anchorHit.name.Contains("WALL"))
            {
                var distance = Vector3.Distance(hit.point, _cameraRig.centerEyeAnchor.position);
                print("Distance to wall: " + distance);
                //get angle from user 0 ,0,0 to hit point
                var angle = Vector3.Angle(Vector3.forward, hit.point - _cameraRig.centerEyeAnchor.position);
                print("Angle to wall: " + angle);
                //_mqttReceiver.sendDistance(distance, angle);
            }
            else if (anchorHit.name.Contains("TABLE"))
            {
                var distance = Vector3.Distance(hit.point, _cameraRig.centerEyeAnchor.position)-distanceOffset;
                print("Distance to ceiling: " + distance);
                //get angle from user 0 ,0,0 to hit point
                var angle = Vector3.Angle(Vector3.forward, hit.point - _cameraRig.centerEyeAnchor.position);
                print("Angle to ceiling: " + angle);
                /*mqttReceiver.sendDistance(distance, angle);*/
            }
            else
            {

                print("no hits");

            }
            ShowHitNormal(hit.point, hit.normal);


        }


    }




}
