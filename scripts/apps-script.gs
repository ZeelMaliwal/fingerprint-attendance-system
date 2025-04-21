function doGet(e) {
  var ss = SpreadsheetApp.getActiveSpreadsheet();
  
  if (!e || !e.parameter || !e.parameter.action) {
    // Return full spreadsheet content
    var sheets = ss.getSheets();
    var output = "";
    
    sheets.forEach(function(sheet) {
      var data = sheet.getDataRange().getValues();
      output += sheet.getName() + "\n";
      output += data.map(row => row.join(", ")).join("\n");
      output += "\n\n";
    });
    
    return ContentService.createTextOutput(output);
  }

  // Handle specific actions
  var action = e.parameter.action;
  try {
    switch (action) {
      case 'enroll':
        var sheet = ss.getSheetByName('Enrollments') || ss.insertSheet('Enrollments');
        var rollNo = e.parameter.data1 || "N/A";
        sheet.appendRow([new Date(), rollNo]);
        break;

      case 'subject':
        var sheet = ss.getSheetByName('Subjects') || ss.insertSheet('Subjects');
        var subjectCode = e.parameter.data1 || "N/A";
        sheet.appendRow([new Date(), subjectCode]);
        break;

      case 'attendance':
        var sheet = ss.getSheetByName('Attendance') || ss.insertSheet('Attendance');
        var fingerprintID = e.parameter.data1 || "N/A";
        var subjectCode = e.parameter.data2 || "N/A";
        var timestamp = e.parameter.data3 || new Date();
        sheet.appendRow([new Date(), fingerprintID, subjectCode, timestamp]);
        break;

      default:
        return ContentService.createTextOutput("Error: Invalid action");
    }

    return ContentService.createTextOutput("Success");
  } catch (error) {
    return ContentService.createTextOutput("Error: " + error.toString());
  }
}