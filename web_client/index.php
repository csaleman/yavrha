<html xmlns="http://www.w3.org/1999/xhtml" > 
<head> 
	<title>Page Title</title> 
	
	<meta name="viewport" content="width=device-width, initial-scale=1"> 

	
    <link rel="stylesheet" href="http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.css" />
    <script src="http://code.jquery.com/jquery-1.9.1.min.js"></script>
    <script src="http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.js"></script>


 	<script type="text/javascript" src="http://www.modernizr.com/downloads/modernizr-latest.js"></script>

	<script type="text/javascript" src="mqttws31.js"></script>
	<script type="text/javascript" src="yavrha.js"></script>

    <style>
        #message {
            display: none;
            border-radius: 10px;
            padding: 10px;
            background-color: #ff0000;
            color: #fff;
        }
    </style>
    <script type="text/javascript">
        

    </script>
</head> 

<body> 

<!-- Start of first page -->
<div data-role="page" id="main">
	
<div data-role="header" data-position="inline">
		<h1>Yavrha Home</h1>
		<a href="#settings" data-icon="gear" class="ui-btn-right">&nbsp;</a>
	</div>
	<!-- /header -->

	<div data-role="content" id="MainUlDiv">	
		<ul data-role="listview" data-inset="true" id="MainUl">

        </ul>		

</div><!-- /content -->

</div><!-- /page -->


<!-- Start of second page -->
<div data-role="page" id="settings">

	<div data-role="header" data-position="inline">
		<a href="#main">Back</a>
		<h1>Settings</h1>
	</div><!-- /header -->

	<div data-role="content">	
		 <p id="message"/>
        <ul data-role="listview" data-inset="true">
            <li data-role="list-divider">MQTT Websocket Server - test.mosquitto.org</li>
            <li> <input type="text" id="MQTTServer" name="MQTTServer" placeholder="Enter MQTT server"/>
            
	    <li data-role="list-divider">MQTT Topic - topic/#</li>
            <li> <input type="text" id="MQTTTopic" name="MQTTTopic" placeholder="Enter MQTT topic"/>
	    <li data-role="list-divider">MQTT Server User</li>
            <li> <input type="text" id="MQTTServerUser" name="MQTTServerUser" placeholder="Enter MQTT server User"/>
	    <li data-role="list-divider">MQTT Server Password</li>
            <li> <input type="text" id="MQTTServerPassword" name="MQTTServerPassword" placeholder="Enter MQTT server Password"/>                 
		
	    <input type="button" id="addToStorage" value="Save"/>
            </li>
        </ul>		
			
	</div><!-- /content -->

	
</div><!-- /page -->
</body>



</body>
</html>

