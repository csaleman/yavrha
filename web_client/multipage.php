
<!DOCTYPE html> 
<html> 
<head> 
	<title>Page Title</title> 
	
	<meta name="viewport" content="width=device-width, initial-scale=1"> 

	<link rel="stylesheet" href="http://code.jquery.com/mobile/1.3.1/jquery.mobile-1.3.1.min.css" />
	<script type="text/javascript" src="http://code.jquery.com/jquery-1.9.1.min.js"></script>
	<script type="text/javascript" src="http://code.jquery.com/mobile/1.3.1/jquery.mobile-1.3.1.min.js"></script>
 	<script type="text/javascript" src="http://www.modernizr.com/downloads/modernizr-latest.js"></script>
	<script type="text/javascript" src="mosquitto-1.1.js"></script>
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
            <li data-role="list-divider">Mosquitto Websocket Server - ws://test.mosquitto.org/mqtt</li>
            <li> <input type="text" id="MosquittoServer" name="MosquittoServer" placeholder="Enter mosquitto server"/>
	    <li data-role="list-divider">Mosquitto Topic - topic/#</li>
            <li> <input type="text" id="MosquittoTopic" name="MosquittoTopic" placeholder="Enter mosquitto topic"/>
	    <li data-role="list-divider">Mosquitto Server User</li>
            <li> <input type="text" id="MosquittoServerUser" name="MosquittoServerUser" placeholder="Enter mosquitto server User"/>
	    <li data-role="list-divider">Mosquitto Server Password</li>
            <li> <input type="text" id="MosquittoServerPassword" name="MosquittoServerPassword" placeholder="Enter mosquitto server Password"/>                 
		
	    <input type="button" id="addToStorage" value="Save"/>
            </li>
        </ul>		
			
	</div><!-- /content -->

	
</div><!-- /page -->
</body>



</body>
</html>

