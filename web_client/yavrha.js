// Global Variables
	var localStorageKey1 = "MosquittoServer";
	var localStorageKey2 = "MosquittoTopic";
 	var localStorageKey3 = "MosquittoServerUser";
	var localStorageKey4 = "MosquittoServerPassword";
	var TOPIC = "";        // Used to store TOPIC
    var NodesObj = {};     // Main object where nodes data is stored
    var OldMSGID = [];   // Array used to store previous MSGID, this is used to avoid to refresh switch etc, back with old information after user change it change.

// Mosquitto Object
	var t = new Mosquitto();

// Nodes Object
// {
//	 "node1":{  "name":"2nd_temp",
//				"type":"1",
//				"address":"230",
//				"number":"1",
//				"msgid":"83",
//				"data0":"19",
//				"data1":"1",
//				"data2":"0",
//				"data3":"0" }
//	"node2": { etc .. .. 

	

//********************************************************
// 		This is the function that initiate jquery.
//********************************************************

        $('#main').live('pageinit', function() {
//		Check if local storage is suported in the browser           
			if (Modernizr.localstorage) {
                showStoreValue();
				MosquittoSetup(localStorage.getItem(localStorageKey1),localStorage.getItem(localStorageKey2));
            }
            else {
                $('#message').text("Unfortunately your browser doesn't support local storage");
                $('#addToStorage').attr('disabled', 'disabled');
                $('#message').show();
            }
           
	        $('#addToStorage').click(function(e) {
//				Save values into localstorage                
				localStorage.setItem(localStorageKey1, $('#MosquittoServer').val());
				localStorage.setItem(localStorageKey2, $('#MosquittoTopic').val());
				localStorage.setItem(localStorageKey3, $('#MosquittoServerUser').val());
				localStorage.setItem(localStorageKey4, $('#MosquittoServerPassword').val());
//				Show Refresh values in page.
				showStoreValue();
//				Prevent Default action. In this case submit form.
                e.preventDefault();
//				Reload page with new settings.
				location.reload();

            });
        });

//********************************************************
// Function to get values from browser's local storage
//********************************************************
function showStoreValue() {
// Mosquitto Server
        	
		$('#MosquittoServer').val(localStorage.getItem(localStorageKey1));
       
// Mosquitto Topic
	var item2 = localStorage.getItem(localStorageKey2);
        	
		$('#MosquittoTopic').val(item2);

// Load Topic in global variable TOPIC		
		TOPIC = item2;           

// Mosquitto Server User
	var item3 = localStorage.getItem(localStorageKey3);
        
		$('#MosquittoServerUser').val(item3);

// Mosquitto Server Password
	var item4 = localStorage.getItem(localStorageKey4);
        		
		$('#MosquittoServerPassword').val(item4);
}

//********************************************************
// This function configure the mosquitto instance with a server and topic.
//********************************************************
function MosquittoSetup(server,topic)
{
   if (server != null && topic != null) {
    	t.connect(server,100000);
    	t.subscribe(topic,0);
// Set callback function
		t.onmessage = StoreMsg;
	}

}
//********************************************************
// This function is a callback from the mosquitto instance
//********************************************************
function StoreMsg(Topic, Payload, qos, retain){
    

//  alert(topic);
	SubMsg = document.getElementById('SubMsg');
	
//  Split topic to get Node Number ej. "topic/node1/name"

	TopicArray = Topic.split('/');
	NodeNumber = TopicArray[1];
	MsgName	   = TopicArray[2];
	

//	cannot add value in an object that doesn't exist yet.
//	this create an emptly object for the node number if it doesn't exist

	if (typeof NodesObj[NodeNumber]=='undefined'){
		
		NodesObj[NodeNumber] = {};
		NodesObj[NodeNumber][MsgName] = Payload;
	}
	else {
		
//		Save the value "Payload" in the property "MsgName" for a given Node

            NodesObj[NodeNumber][MsgName] = Payload;

    }

    



//	If message is "Number" which is the last configuration message sent by the python client call the PaintScreen function.

	if (MsgName == "number") {
		
		PaintScreen();
	}
            
        UpdateValues();

       
	 //DebugWindow.value = JSON.stringify(NodesObj);
	 //console.log(NodesObj);	

}

//********************************************************
// This function create  the custom forms widgets
//********************************************************
function PaintScreen()
{
//		Remove all li in listview
		$('ul#MainUl').children().remove('li');
//		Print a debug text area       
//		$('ul#MainUl').append('<li data-role="list-divider">Debug Window</li><li> <textarea rows="5" cols="25" id="DebugWindow"></textarea>');

	for (var NodeNumber in NodesObj) {

		Node = NodesObj[NodeNumber];
		
		if (Node["type"] > 127) {
			
			switch (parseInt(Node["type"])){
				
				case 128:
					$('ul#MainUl').append('<li data-role="list-divider">Switch ' + Node["name"] + ' </li>');
			 		$('ul#MainUl').append('<li><select name="node'+ Node["number"] + '" id="node'+ Node["number"] + '" data-role="slider" data-mini="true"> \
										<option value="0">Off</option> <option value="1">On \
										</option> </select></li>');
					$('#node'+ Node["number"] ).on( 'slidestop', function( event ) { PostSwitchMessage($(this).attr('id') ,$(this).attr('value')); });
					break;
				case 129:
					$('ul#MainUl').append('<li data-role="list-divider">Dimmer ' + Node["name"] + ' </li>');
			 		$('ul#MainUl').append('<li> <input type="text" id="Node1" name="Node1"/> </li>');
					break;
				case 130:
					$('ul#MainUl').append('<li data-role="list-divider">Servo ' + Node["name"] + ' </li>');
			 		$('ul#MainUl').append('<li> <input type="text" id="Node1" name="Node1"/> </li>');
					break;
					

			}
		} 
	}
		
	for (var NodeNumber in NodesObj) {
		
		Node = NodesObj[NodeNumber];
		
		if (Node["type"] < 128) {
			
			switch (parseInt(Node["type"])){
				
				case 1:
					$('ul#MainUl').append('<li data-role="list-divider">Temperature ' + Node["name"] + ' </li>');
			 		$('ul#MainUl').append('<li id="node'+ Node["number"]+'" >00.0</li>');
					break;
				case 2:
					$('ul#MainUl').append('<li data-role="list-divider">Status ' + Node["name"] + ' </li>');
			 		$('ul#MainUl').append('<li> <input type="text" id="Node1" name="Node1"/> </li>');
					break;
				case 3:
					$('ul#MainUl').append('<li data-role="list-divider">Power ' + Node["name"] + ' </li>');
			 		$('ul#MainUl').append('<li> <input type="text" id="Node1" name="Node1"/> </li>');
					break;
					
			}
		}
	}



// 	This refresh the CSS in the list.
//	This works in JQ Mobile 1.3+
//	$('#MainUl').listview("refresh");
// 	This works in all JQ Mobile Version 	 
	$('#MainUl').listview("refresh").trigger("create");
}


//********************************************************
// Function to update values
//********************************************************
function UpdateValues(){

		for (var NodeNumber in NodesObj) {
						
			Node = NodesObj[NodeNumber];

			switch (parseInt(Node["type"])){
				
				case 1:
					var sign = (Node["data1"] = 1) ? "" : "-";
					var tempC = parseInt(sign + Node["data0"] );
					var tempF = Math.round((tempC*1.8)+32);
					$('#'+NodeNumber).text(tempC + "  \u00B0C ( " + tempF + " \u00B0F )"  );	
					break;
				case 2:
					
					break;
				case 3:
					
					break;
				case 128:
                    // Only refresh data if the MSGID is new.
                    // This is important to keep the slider steady in OFF or ON after a user change it status.                    
                        // this first if, just initialize the first time.
                        if (typeof OldMSGID[NodeNumber] == 'undefined' && typeof Node["msgid"] != 'undefined' ){
                                OldMSGID[NodeNumber] = parseInt(Node["msgid"]) -1 ;
                                console.log(OldMSGID[NodeNumber]);
                                console.log(Node["msgid"]);
                            }
                        console.log("Node msgid: " + Node["msgid"]);
                        console.log("Old msgid: " + OldMSGID[NodeNumber]);
                        if(OldMSGID[NodeNumber] < Node["msgid"]) {			
                        OldMSGID[NodeNumber] = parseInt(Node["msgid"]);
                    // This command assign a new value to the widget and refresh it.                        
                        $('#'+NodeNumber).val(Node["data0"]).slider("refresh");

                        }
					break;
				case 129:
					
					break;
				case 130:
					
					break;


			}
		}
}

  
//********************************************************
// Function to Post Messages back to MQTT server
//********************************************************
function PostSwitchMessage(Node, PostVal)
{
// Increment OldMSGID by 2, which means do not use the next X messages, this is to keep the slider steady in position after a user update    
    OldMSGID[NodeNumber]  =  parseInt(OldMSGID[NodeNumber]) + 2;

    if (OldMSGID[NodeNumber] > 254) {
        OldMSGID[NodeNumber] = 0
    }    
    postTopic = TOPIC.substring(0, TOPIC.length - 1) + Node + "/cmd";
    t.publish(postTopic, PostVal,0,0);

	//alert(postTopic);
}
