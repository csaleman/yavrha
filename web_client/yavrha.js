/*
               Yavrha
     Copyright (C) Carlos Silva, 2013
      (csaleman [at] gmail [dot] com)

      http://code.google.com/p/yavrha/
*/

/*
    This file is part of Yavrha.

    Yavrha is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Yavrha is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yavrha.  If not, see <http://www.gnu.org/licenses/>.

*/


// Global Variables
	var localStorageKey1 = "MosquittoServer";
	var localStorageKey2 = "MosquittoTopic";
 	var localStorageKey3 = "MosquittoServerUser";
	var localStorageKey4 = "MosquittoServerPassword";
	var TOPIC = "";        // Used to store TOPIC
    var NodesObj = {};     // Main object where nodes data is stored

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

        $(document).on('pageinit',"#main", function() {
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
// This function Refresh the values every 10 seconds.
//********************************************************

window.setInterval(RefreshFunctions, 10000);

function RefreshFunctions() { 
    
//        alert('test'); 

//      Update all controls values
        UpdateValues();ko khnnjhm ;n; n., l;loh ;lkp[];def,gr';hpk UpdateValues();][;ytki45g ;'[6-yto0iytihjn'lN
'[t;l;jlkmhlk,lkh
hl';hloigi9ut8598 t79otlgkjvokg0go0oggpovl l n,mb,bfl,bkl,m


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
//  Make sure of update all controls values after PaintScreen.
        
        UpdateValues();
	}
        
//  Update Data only if a data0 to data3 is posted.

     if (MsgName == "data0" || MsgName == "data1" || MsgName == "data2" || MsgName == "data3") {    
       
         UpdateValues();
     }
       
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
				// Device type 128 On/Off device
				case 128:
					$('ul#MainUl').append('<li data-role="list-divider">Switch ' + Node["name"] + ' </li>');
			 		$('ul#MainUl').append('<li><select name="node'+ Node["number"] + '" id="node'+ Node["number"] + '" data-role="slider" data-mini="true"> \
										   <option value="0">Off</option> <option value="1">On\
										   </option> </select></li>');
					$('#node'+ Node["number"] ).on( 'slidestop', function( event ) { PostSwitchMessage($(this).attr('id') ,$(this).val()); });		
                    break;
            	
                // Device type 129 On/Off device with Dimmer		
            	case 129:
					$('ul#MainUl').append('<li data-role="list-divider">Dimmer ' + Node["name"] + ' </li>');
			 		$('ul#MainUl').append('<li><select name="Snode'+ Node["number"] + '" id="Snode'+ Node["number"] + '" data-role="slider" data-mini="true"> \
										   <option value="0">Off</option> <option value="1">On\
										   </option> </select>\
                                           <label for="node'+ Node["number"] + '" class="ui-hidden-accessible">Input slider:</label> \
                                           <input type="range" name="node'+ Node["number"] + '" id="node'+ Node["number"] + '" \
                                           value="5" min="0" max="10" step="1" data-highlight="true" data-mini="true"/></li>');
                    
//                	Refresh the CSS in the list. The event handlers MUST be set after refreshing the list. 
                    $('#MainUl').listview("refresh").trigger("create"); 
//                  Set event handler to handle dimmer                                       
                    $('#node'+ Node["number"]).ready( CreateDimmerEvent(Node["number"]));
//                  Set event handler to hande switch
                    $('#Snode'+ Node["number"]).ready(  CreateDimmerSwitchEvent(Node["number"]));
                 
                    break;
                
                // Device type 130 Servo Device 				
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
// This function create event handlers for Dimmer Slider
// 
//********************************************************
function CreateDimmerEvent(NodeNumber){
  
// The following check if the handler exist before creating one.
    var ev = $._data($('#node'+ NodeNumber)[0], 'events');

    if (!(ev && ev.slidestop)) {
//  Create a event handler        
            $('#node'+ NodeNumber).on('slidestop', function( event ) { PostDimmerMessage($(this).attr('id'));  });
        }

}

//********************************************************
// This function create event handlers for Dimmer Switch
//********************************************************
function CreateDimmerSwitchEvent(NodeNumber){
  
// The following check if the handler exist before creating one.
    var ev = $._data($('#Snode'+ NodeNumber)[0], 'events');

    if (!(ev && ev.slidestop)) {
//  Create a event handler        
            $('#Snode'+ NodeNumber).on('slidestop', function( event ) { PostDimmerMessage('node'+ NodeNumber); });
        }

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
                    // This command assign a new value to the widget and refresh it.                        
                        $('#'+NodeNumber).val(Node["data0"]).slider("refresh");
					break;
				case 129:
                    // This command assign a new value to the widget and refresh it.                        
                        $('#S'+NodeNumber).val(Node["data0"]).slider("refresh");
                        $('#'+NodeNumber).val(parseInt(Node["data1"]) / 25).slider("refresh");

					break;
				case 130:
					
					break;


			}
		}
}

  
//***************************************************************************************
// Function to Post Messages back to MQTT server from Device Type 128 On/OFF Switch
//***************************************************************************************
function PostSwitchMessage(Node, PostVal)
{

    postTopic = TOPIC.substring(0, TOPIC.length - 1) + Node + "/cmd";
    t.publish(postTopic, PostVal,0,0);

	//alert(postTopic);
}

//*********************************************************************************************
// Function to Post Messages back to MQTT server from Device Type 129 On/OFF Switch with dimmer
// arguments: node
//  Function will look for values in Snode for switch on/off and in node for dimmer pwm
//*********************************************************************************************
function PostDimmerMessage(Node)
{
   //Since slider will step from 1 to 10, I will multiply the value by 25
    var newPostVal = parseInt( $('#'+Node).val()) * 25;

    postTopic = TOPIC.substring(0, TOPIC.length - 1) + Node + "/cmd";
    t.publish(postTopic, $('#S'+Node).val() + " " + newPostVal,0,0);

	//alert(postTopic);
}
