import redis
import json

# Redis server connection details
REDIS_HOST = "10.151.99.181"  # Replace with your Redis server's IP
REDIS_PORT = 6379  # Redis port
REDIS_CHANNEL = "Detection::yolonas::0"  # Replace with your Redis channel name

def process_message(message):
    try:
        # Parse the JSON string into a Python dictionary
        data = json.loads(message)
        
        # Extract the "object_detection" list
        objects = data.get("object_detection", [])
        
        # Filter objects labeled "person"
        persons = [obj for obj in objects if obj.get("label") == "person"]
        
        # Print each "person" object
        for person in persons:
            print(json.dumps(person, indent=4))
    
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")

def listen_to_redis():
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

# Start listening
listen_to_redis()
