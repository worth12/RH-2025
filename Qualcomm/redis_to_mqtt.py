import redis
import json
import time
import paho.mqtt.client as mqtt

# Redis server connection details
REDIS_HOST = "10.151.99.181"  # Replace with your Redis server's IP
REDIS_PORT = 6379  # Redis port
REDIS_CHANNEL = "Detection::yolonas::0"  # Replace with your Redis channel name

# MQTT server connection details
MQTT_BROKER = "10.151.99.211"  # Replace with your MQTT broker address
MQTT_PORT = 1883  # MQTT port
MQTT_TOPIC = "detection/person"  # Replace with your MQTT topic

# Timer interval in seconds
SEND_INTERVAL = 0.05
last_send_time = 0  # Tracks the last time a message was sent

# Initialize MQTT client
mqtt_client = mqtt.Client()

def connect_mqtt():
    """Connect to the MQTT broker."""
    try:
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT)
        print(f"Connected to MQTT broker at {MQTT_BROKER}:{MQTT_PORT}")
    except Exception as e:
        print(f"Failed to connect to MQTT broker: {e}")
        exit(1)

def process_message(message):
    """Process Redis message and publish 'person' objects over MQTT."""
    global last_send_time
    
    try:
        # Parse the JSON string into a Python dictionary
        data = json.loads(message)
        
        # Extract the "object_detection" list
        objects = data.get("object_detection", [])
        
        # Filter objects labeled "person"
        persons = [obj for obj in objects if obj.get("label") == "person"]
        
        # Get the current time
        current_time = time.time()
        
        # Check if enough time has passed since the last send
            # Publish each "person" object to MQTT
        for person in persons:
            if current_time - last_send_time >= SEND_INTERVAL:
                mqtt_client.publish(MQTT_TOPIC, json.dumps(person))
                print(f"Published to MQTT: {json.dumps(person, indent=4)}")
                # Update the last send time
                last_send_time = current_time
    
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

def listen_to_redis():
    """Subscribe to Redis channel and process messages."""
    # Connect to the Redis server
    client = redis.StrictRedis(host=REDIS_HOST, port=REDIS_PORT, decode_responses=True)
    
    # Subscribe to the Redis channel
    pubsub = client.pubsub()
    pubsub.subscribe(REDIS_CHANNEL)
    
    print(f"Listening to Redis channel: {REDIS_CHANNEL}")
    
    # Listen for messages
    for message in pubsub.listen():
        if message["type"] == "message":
            process_message(message['data'])

# Connect to MQTT and start listening to Redis
connect_mqtt()
listen_to_redis()