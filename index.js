function test(event) {
    let commandValue = event.target.value.getUint8(0);
    write = document.getElementById('value');
    write.innerHTML = commandValue;
    console.log(commandValue);
}

function onRequestBluetoothDeviceButtonClick() {

        const controlServiceUUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b'; // Full UUID
        const commandCharacteristicUUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8';


        navigator.bluetooth.requestDevice({
            acceptAllDevices: true,
            optionalServices: [controlServiceUUID]
        })

        .then(device => {
            console.log("Got device name: ", device.name);
            console.log("id: ", device.id);
            return device.gatt.connect();
        })

        .then(server => {
            // Step 3: Get the Service
            serverInstance = server;
            console.log("Getting PrimaryService");
            return server.getPrimaryService(controlServiceUUID);
        })

        .then(service => {
            console.log("Getting Characteristic");
            return service.getCharacteristic(commandCharacteristicUUID);
        })

        .then (characteristic => {

            characteristic.startNotifications();
            characteristic.addEventListener('characteristicvaluechanged', test);

        })

        .catch(function(error) {
            console.log("Something went wrong. " + error);
        });



   }



