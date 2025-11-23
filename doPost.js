function doPost(e) {
  try {
    var sheet = SpreadsheetApp.openById('15cVD-PQSRoMAoWLANx6Bgpv8LjGBc5tTnS37DZrOWYg').getActiveSheet();
    var data = JSON.parse(e.postData.contents);
    var serverReceiveTime = new Date();
    
    console.log('Server receive time:', serverReceiveTime);
    console.log('Received data structure:', data);
    
    var batchSize = data.batch_size;
    var readings = data.readings;
    
    if (readings && readings.length > 0) {
      // Find the maximum millisFromStart in this batch
      var maxMillis = 0;
      for (var i = 0; i < readings.length; i++) {
        if (readings[i].millisFromStart > maxMillis) {
          maxMillis = readings[i].millisFromStart;
        }
      }
      
      console.log('Max millis in batch:', maxMillis);
      
      for (var i = 0; i < readings.length; i++) {
        var reading = readings[i];
        
        // Calculate how long ago this reading was taken
        var timeOffset = maxMillis - reading.millisFromStart;
        
        // Subtract from server receive time to get actual reading time
        var estimatedTime = new Date(serverReceiveTime.getTime() - timeOffset);
        
        console.log('Reading', i, 'millisFromStart:', reading.millisFromStart, 
                   'offset:', timeOffset, 'estimated:', estimatedTime);
        
        sheet.appendRow([
          estimatedTime,
          reading.t0,
          reading.t1,
          reading.t2
        ]);
      }
      console.log('Successfully wrote ' + readings.length + ' rows to sheet.');
      return ContentService.createTextOutput(JSON.stringify({
        status: 'success', 
        message: 'Processed ' + readings.length + ' readings',
        server_time: serverReceiveTime.toString(),
        max_millis: maxMillis
      })).setMimeType(ContentService.MimeType.JSON);
    } else {
      console.error('No "readings" array found in payload');
      return ContentService.createTextOutput(JSON.stringify({status: 'error', message: 'No readings array found'})).setMimeType(ContentService.MimeType.JSON);
    }
    
  } catch (error) {
    console.error('Full Error:', error);
    return ContentService.createTextOutput(JSON.stringify({status: 'error', message: error.toString()})).setMimeType(ContentService.MimeType.JSON);
  }
