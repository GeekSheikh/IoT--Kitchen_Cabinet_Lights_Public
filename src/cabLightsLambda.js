var http = require('https');

var kitchenCabinetLEDs = "5fd4da3e-db76-4554-9d5a-978b3be044f9";

var particleServer = "api.particle.io";
var particlePath = "/v1/devices/";

/**
 * Main entry point.
 * Incoming events from Alexa Lighting APIs are processed via this method.
 */
exports.handler = function(event, context) {

    log('Input', event);

    switch (event.header.namespace) {

        /**
         * The namespace of "Discovery" indicates a request is being made to the lambda for
         * discovering all appliances associated with the customer's appliance cloud account.
         * can use the accessToken that is made available as part of the payload to determine
         * the customer.
         */
        case 'Alexa.ConnectedHome.Discovery':
            handleDiscovery(event, context);
            break;

            /**
             * The namespace of "Control" indicates a request is being made to us to turn a
             * given device on, off or brighten. This message comes with the "appliance"
             * parameter which indicates the appliance that needs to be acted on.
             */
        case 'Alexa.ConnectedHome.Control':
            handleControl(event, context);
            break;

            /**
             * We received an unexpected message
             */
        default:
            log('Err', 'No supported namespace: ' + event.header.namespace);
            context.fail('Something went wrong');
            break;
    }
};

/**
 * This method is invoked when we receive a "Discovery" message from Alexa Connected Home Skill.
 * We are expected to respond back with a list of appliances that we have discovered for a given
 * customer.
 */
function handleDiscovery(accessToken, context) {

    /**
     * Crafting the response header
     */
    var headers = {
        namespace: 'Alexa.ConnectedHome.Discovery',
        name: 'DiscoverAppliancesResponse',
        payloadVersion: '2'
    };

    /**
     * Response body will be an array of discovered devices.
     */
    var appliances = [];

    var kitchenLEDLights = {
        applianceId: kitchenCabinetLEDs,
        manufacturerName: 'srfnmnk',
        modelName: 'LEDLights',
        version: 'VER01',
        friendlyName: 'Kitchen LED Lights',
        friendlyDescription: 'Lights underneath Cabinets in Kitchen Raleigh',
        isReachable: true,
        actions:[
            "incrementPercentage",
            "decrementPercentage",
            "setPercentage",
            "turnOn",
            "turnOff"
        ],
        additionalApplianceDetails: {
            /**
             * OPTIONAL:
             * We can use this to persist any appliance specific metadata.
             * This information will be returned back to the driver when user requests
             * action on this appliance.
             */
            fullApplianceId: '5fd4da3e-db76-4554-9d5a-978b3be044f9',
            deviceId: "<REDACTED>"
        }
    };

    appliances.push(kitchenLEDLights);

    /**
     * Craft the final response back to Alexa Connected Home Skill. This will include all the
     * discoverd appliances.
     */
    var payloads = {
        discoveredAppliances: appliances
    };
    var result = {
        header: headers,
        payload: payloads
    };

    log('Discovery', result);

    context.succeed(result);
}

/**
 * Control events are processed here.
 * This is called when Alexa requests an action (IE turn off appliance).
 */
function handleControl(event, context) {
    if (event.header.namespace === 'Alexa.ConnectedHome.Control') {

        /**
         * Retrieve the appliance id and accessToken from the incoming message.
         */
        var accessToken = event.payload.accessToken;
        var applianceId = event.payload.appliance.applianceId;
        var deviceid = event.payload.appliance.additionalApplianceDetails.deviceId;
        var message_id = event.header.messageId;
        var param = "";
        var state = 0;
        var confirmation;
        var funcName;

        log("Access Token: ", accessToken);
        log("DeviceID: ", deviceid);
        log("Event Header Name: ", event.header.name);

        if(event.header.name == "TurnOnRequest"){
            state = 1;
            confirmation = "TurnOnConfirmation";
            funcName = "onoff";
        }
        else if(event.header.name == "TurnOffRequest"){
            state = 0;
            confirmation = "TurnOffConfirmation";
            funcName = "onoff";
        }
        else if(event.header.name == "SetPercentageRequest"){
            state = event.payload.percentageState.value;
            confirmation = "SetPercentageConfirmation";
            funcName = "setvalue";
        }
        else if(event.header.name == "IncrementPercentageRequest"){
            var increment = event.payload.deltaPercentage.value;

            state = increment;

            confirmation = "IncrementPercentageConfirmation";
            funcName = "deltachange";
        }
        else if(event.header.name == "DecrementPercentageRequest"){
            var decrement = event.payload.deltaPercentage.value;

            state = -decrement;

            confirmation = "DecrementPercentageConfirmation";
            funcName = "deltachange";
        }

        log('applianceId', applianceId);

        param = state;

        var options = {
          "method": "POST",
          "hostname": "api.particle.io",
          "port": null,
          "path": "/v1/devices/<REDACTED>/" + funcName,
          "headers": {
              "content-type": "application/x-www-form-urlencoded",
              "authorization": "<REDACTED>",
              "cache-control": "no-cache",
              "postman-token": "<REDACTED>"
            }
          };

        log(options);

        var data = "args=" + param;

        log(data);

        var serverError = function (e) {
            log('Error', e.message);
            context.fail(generateControlError('TurnOnRequest', 'DEPENDENT_SERVICE_UNAVAILABLE', 'Unable to connect to server'));
        };

        var callback = function(response) {
            var str = '';

            response.on('data', function(chunk) {
                str += chunk.toString('utf-8');
            });

            response.on('end', function() {
                log('Return Value');
                log(str);

                var headers = {
                    namespace: 'Alexa.ConnectedHome.Control',
                    name: confirmation,
                    payloadVersion: '2',
                    messageId: message_id
                };
                var payloads = {

                };
                var result = {
                    header: headers,
                    payload: payloads
                };

                context.succeed(result);
            });

            response.on('error', serverError);
        };

        var req = http.request(options, callback);

        req.on('error', serverError);

        req.write(data);
        req.end();
    }
}

/**
 * Utility functions.
 */
function log(title, msg) {
    console.log(title + ": " + msg);
}

function generateControlError(name, code, description) {
    var headers = {
        namespace: 'Control',
        name: name,
        payloadVersion: '1'
    };

    var payload = {
        exception: {
            code: code,
            description: description
        }
    };

    var result = {
        header: headers,
        payload: payload
    };

    return result;
}
