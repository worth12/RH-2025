using M2MqttUnity;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using uPLibrary.Networking.M2Mqtt.Messages;
using System.Threading.Tasks;
using UnityEngine.SceneManagement;
using Newtonsoft.Json;
using System;
using UnityEngine.UI;
using TMPro;

public class mqttReceiver : M2MqttUnityClient
{
    [System.Serializable]
    public class EnvironmentalData
    {
        public float temperature;    // in Celsius
        public float humidity;       // in percentage
        public float pressure;       // in hPa
        public float altitude;       // in meters
        public float pm1_0;          // in ?g/m^3
        public float pm2_5;          // in ?g/m^3
        public string pm1_0Status;   // PM1.0 status (e.g. "Good")
        public string pm2_5Status;   // PM2.5 status (e.g. "Clean")
        public float Tvoc_indx;      // TVOC index

        // Constructor to initialize the values
        public EnvironmentalData(float temperature, float humidity, float pressure, float altitude, float pm1_0, float pm2_5, string pm1_0Status, string pm2_5Status, float Tvoc_indx)
        {
            this.temperature = temperature;
            this.humidity = humidity;
            this.pressure = pressure;
            this.altitude = altitude;
            this.pm1_0 = pm1_0;
            this.pm2_5 = pm2_5;
            this.pm1_0Status = pm1_0Status;
            this.pm2_5Status = pm2_5Status;
            this.Tvoc_indx = Tvoc_indx;
        }
    }
    [Serializable]
    public class Detection
    {
        public string label;        // Label of the detected object
        public float confidence;    // Confidence value of the detection
        public int color;           // Color represented as an integer
        public Rectangle rectangle; // Rectangle object for bounding box

        [Serializable]
        public class Rectangle
        {
            public float x;      // X-coordinate (normalized)
            public float y;      // Y-coordinate (normalized)
            public float width;  // Width of the bounding box (normalized)
            public float height; // Height of the bounding box (normalized)
        }
    }

    public static mqttReceiver _mqttReceiver;
    [Header("MQTT topics")]
    [Tooltip("Set the topic to subscribe. !!!ATTENTION!!! multi-level wildcard # subscribes to all topics")]
    // topic to subscribe. !!! The multi-level wildcard # is used to subscribe to all the topics. Attention i if #, subscribe to all topics. Attention if MQTT is on data plan

    string[] topicSubscribe = { "all_sensor", "detection/person","detection/fire" };
    byte[] qosLevelsSubscribe = { MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE, MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE, MqttMsgBase.QOS_LEVEL_EXACTLY_ONCE };
    [Tooltip("Set the topic to publish (optional)")]
    public string topicPublishDistance = "rover/distance"; // topic to publish distance
    public string topicPublishAngle = "rover/move"; // topic to publish angle
    public string topicPublishStick = "rover/stick"; // topic to publish joystick direction

    //using C# Property GET/SET and event listener to reduce Update overhead in the controlled objects
    private string m_msg;

    public TMP_Text senseDataCanvas;
    public TMP_Text detectDataCanvas;
    public TMP_Text FireDataCanvas;


    public string msg
    {
        get
        {
            return m_msg;
        }
        set
        {
            if (m_msg == value) return;
            m_msg = value;
            if (OnMessageArrived != null)
            {
                OnMessageArrived(m_msg);
            }
        }
    }

    private void OnEnable()
    {

    }

    protected override void Start()
    {
        if (_mqttReceiver == null)
        {
            _mqttReceiver = this;
        }
        base.Start();



    }

    public event OnMessageArrivedDelegate OnMessageArrived;
    public delegate void OnMessageArrivedDelegate(string newMsg);

    //using C# Property GET/SET and event listener to expose the connection status
    private bool m_isConnected;

    public bool isConnected
    {
        get
        {
            return m_isConnected;
        }
        set
        {
            if (m_isConnected == value) return;
            m_isConnected = value;
            if (OnConnectionSucceeded != null)
            {
                OnConnectionSucceeded(isConnected);
            }
        }
    }
    public event OnConnectionSucceededDelegate OnConnectionSucceeded;
    public delegate void OnConnectionSucceededDelegate(bool isConnected);

    // a list to store the messages
    private List<string> eventMessages = new List<string>();
    private Dictionary<string, string> MQTTMessages = new Dictionary<string, string>();


    public void Publish(string _topic, string msg)
    {
        if (client == null)
            return;
        client.Publish(
            _topic, System.Text.Encoding.UTF8.GetBytes(msg),
            MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE, false);
        Debug.Log("message published: " + _topic + "--" + msg);
    }

    public void PublishLWT(string _topic, string msg)
    {
        if (client == null)
            return;
        client.Publish(
            _topic, System.Text.Encoding.UTF8.GetBytes(msg),
            MqttMsgBase.QOS_LEVEL_AT_MOST_ONCE, false);
    }
    public void SetEncrypted(bool isEncrypted)
    {
        this.isEncrypted = isEncrypted;
    }

    protected override void OnConnecting()
    {
        base.OnConnecting();
    }


    protected override void OnConnected()
    {
        base.OnConnected();
        isConnected = true;
        PublishLWT("USER" + clientId, "Online");
        //sendDistance(3);
        //Task.Run(() => PersistConnectionAsync());
        //SubscribeTopics();

    }

    protected override void OnConnectionFailed(string errorMessage)
    {
        Debug.Log("CONNECTION FAILED! " + errorMessage);
    }

    protected override void OnDisconnected()
    {
        Debug.Log("Disconnected.");
        isConnected = false;
    }

    protected override void OnConnectionLost()
    {
        Debug.Log("CONNECTION LOST!");

    }

    static bool _tryReconnectMQTT = true;
    private async Task PersistConnectionAsync()
    {
        var connected = client.IsConnected;
        while (_tryReconnectMQTT)
        {
            //Debug.Log("Persisting MQTT connection...");
            if (!connected)
            {
                try
                {
                    Debug.Log("Reconnecting...");
                    base.Connect();
                }
                catch
                {
                    Debug.Log("failed reconnect");
                }
            }
            await Task.Delay(5000);
            connected = client.IsConnected;
        }
    }

    protected override void SubscribeTopics()
    {
        if (client == null)
            return;
        client.Subscribe(topicSubscribe, qosLevelsSubscribe);
        print("Subscribed to: " + topicSubscribe);
    }

    protected override void UnsubscribeTopics()
    {
        if (client == null)
            return;
        client.Unsubscribe(topicSubscribe);
    }

    protected override void DecodeMessage(string topic, byte[] message)
    {
        Debug.Log("Received message from " + topic);
        //The message is decoded
        switch (topic)
        {
            case "all_sensor":
                Debug.Log("Received message from " + topic);
                msg = System.Text.Encoding.UTF8.GetString(message);
                if (msg != null)
                {
                    EnvironmentalData data = JsonConvert.DeserializeObject<EnvironmentalData>(msg);
                    Debug.Log("Temperature: " + data.temperature + "C");
                    Debug.Log("Humidity: " + data.humidity + "%");
                    Debug.Log("Pressure: " + data.pressure + "hPa");
                    Debug.Log("Altitude: " + data.altitude + "m");
                    Debug.Log("PM1.0: " + data.pm1_0 + "g/m^3");
                    Debug.Log("PM2.5: " + data.pm2_5 + "g/m^3");
                    Debug.Log("PM1.0 Status: " + data.pm1_0Status);
                    Debug.Log("PM2.5 Status: " + data.pm2_5Status);
                    //set this to dataCanvas
                    senseDataCanvas.text = "Temperature: " + data.temperature + "C\n" +
                        "Humidity: " + data.humidity + "%\n" +
                        "Pressure: " + data.pressure + "hPa\n" +
                        "Altitude: " + data.altitude + "m\n" +
                        "PM1.0: " + data.pm1_0 + "g/m^3\n" +
                        "PM2.5: " + data.pm2_5 + "g/m^3\n" +
                        "PM1.0 Status: " + data.pm1_0Status + "\n" +
                        "PM2.5 Status: " + data.pm2_5Status;
                }

                break;
            case "detection/person":
                Debug.Log("Received message from " + topic);
                msg = System.Text.Encoding.UTF8.GetString(message);
                if (msg != null)
                {
                    if (msg == " ")
                    {
                        detectDataCanvas.gameObject.SetActive(false); ;
                    }
                    else
                    {
                        detectDataCanvas.gameObject.SetActive(true);
                    }

                }
                break;
            case "detection/fire":
                Debug.Log("Received message from " + topic);
                msg = System.Text.Encoding.UTF8.GetString(message);
                if (msg != null)
                {
                    if (msg == " ")
                    {
                        FireDataCanvas.gameObject.SetActive(false); ;
                    }
                    else
                    {
                        FireDataCanvas.gameObject.SetActive(true);
                    }
                }
                break;
            default:
                break;


        }
        //Debug.Log("Received: " + msg);
        //Debug.Log("from topic: " + topic);

        //StoreMessage(topic,msg);

    }

    public void sendDistance(float distance, float angle)
    {
        //round to 2 decimal places
        distance = (float)Math.Round(distance, 2);
        angle = (float)Math.Round(angle, 2);
        print("angle sent: " + angle);
        //Publish(topicPublishDistance, (distance.ToString()));
        Publish(topicPublishAngle, (distance.ToString() + "," + angle.ToString()));
        print("Distance sent: " + distance);

    }

    // Update is called once per frame
    protected override void Update()
    {
        base.Update(); // call ProcessMqttEvents()

    }


    //public void buttonOneDown()
    //{
    //    //toggle detectDataCanvas.gameObject
    //    if (detectDataCanvas.gameObject.activeSelf)
    //    {
    //        detectDataCanvas.gameObject.SetActive(false);
    //    }
    //    else
    //    {
    //        detectDataCanvas.gameObject.SetActive(true);
    //    }

    //}

    //public void buttonTwoDown()
    //{
    //    //toggle FireDataCanvas.gameObject
    //    if (FireDataCanvas.gameObject.activeSelf)
    //    {
    //        FireDataCanvas.gameObject.SetActive(false);
    //    }
    //    else
    //    {
    //        FireDataCanvas.gameObject.SetActive(true);
    //    }
    //}


    public void buttonOneDown()
    {
        sendDistance(1,180);

    }

    public void buttonTwoDown()
    {
        sendDistance(1, 180);
    }




}
