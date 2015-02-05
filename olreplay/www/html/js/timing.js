"use strict";

var Page1 = {
  MSG_TYPE: 0,
  MSG_RACE_TIME: 1,
  MSG_TIME_OF_DAY: 2
};

function WebSocketTest() {
  if ("WebSocket" in window) {
    var ws = new WebSocket("ws://localhost:9980/ws");
    ws.onopen = function () {
      ws.send("START RACE");
    };

    ws.onmessage = function (evt) {
      var msg = evt.data;
      document.getElementById("status").innerHTML = "Socket Open";

      for (var n = 1, r = document.querySelector("tbody.raw>tr"), p = r.parentNode; n--; p.appendChild(r.cloneNode(true)));
      var table = document.getElementById("messages");

      if (typeof WebSocketTest.counter == 'undefined')
        WebSocketTest.counter = 0;

      table.rows[table.rows.length - 1].cells[0].innerHTML = ++WebSocketTest.counter;
      table.rows[table.rows.length - 1].cells[1].innerHTML = msg;

      if (table.rows.length >= 25)
        table.deleteRow(1);

      var rows = table.getElementsByTagName("tr");
      table.focus();
      table.parentNode.scrollTop = rows[table.rows.length - 1].offsetTop - rows[0].offsetHeight;

      var $selector = angular.element(ms);
      var $scope = $selector.scope();
      //var controller = selector.controller();
      //var injector = selector.injector();
      if (WebSocketTest.counter == 1)
        $scope.startTimer();

      var msgType = msg.split(",", 1)[0];
      if (msgType == 'com') {
        // Index with com message.
        var COM_GRID_POS = 4;
        var COM_RACE_NUM = 3;
        var COM_SHORT_NAME = 5;
        var COM_NAME = 6;
        var COM_TEAM_NAME = 7;

        var values = msg.split(",");

        $scope.competitors[values[COM_RACE_NUM]] = new Competitor(values[COM_RACE_NUM], values[COM_GRID_POS], values[COM_SHORT_NAME], values[COM_NAME], values[COM_TEAM_NAME], [0, 0, 0], 0, 0);
        $scope.positions[values[COM_GRID_POS] - 1].pos = values[COM_GRID_POS];
        $scope.positions[values[COM_GRID_POS] - 1].num = values[COM_RACE_NUM];
        $scope.positions[values[COM_GRID_POS] - 1].name = values[COM_NAME];
      } else if (msgType == 'sec') {
        ProcessSectorMessage($scope, msg)
      } else if (msgType == 'lap') {
        ProcessLapMessage($scope, msg);
      }

      $scope.$apply();
    };

    ws.onclose = function () {
      document.getElementById("status").innerHTML = "Socket Closed";
    };

    window.onbeforeunload = function () {
      ws.onclose = function () { };
      ws.close();
    };
  }
  else {
    alert("This browser does not support WebSockets.");
  }
}

function CheckRaceOrder($scope, raceNum, lapNum, secNum) {
  var currentPos = GetCurrentPos($scope, raceNum);

  if (lapNum == $scope.competitorCrossedLineOnLap[raceNum])
    return currentPos;

  var newPos = GetNewPos($scope, lapNum, secNum);

  if (currentPos != newPos)
    ChangeRaceOrder($scope.positions, currentPos, newPos);

  return newPos;
}

function ProcessLapMessage($scope, msg) {
  var MSG_LAP_RACE_NUM = 3;
  var MSG_LAP_NUM = 4;
  var MSG_LAP_GAP = 5;
  var MSG_LAP_TIME = 6;

  var values = msg.split(",");

  var raceNum = values[MSG_LAP_RACE_NUM];
  var lapNum = values[MSG_LAP_NUM];
  var raceTime = values[Page1.MSG_RACE_TIME];

  var newPos = CheckRaceOrder($scope, raceNum, lapNum, '3');
  
  //$scope.competitorStyle[newPos - 1] = 'recent';
  //$scope.lapNumStyle[newPos - 1] = 'recent';

  var line = $scope.positions[newPos - 1];
  line.lap = values[MSG_LAP_TIME];
  line.lapNum = lapNum;
  line.raceTime = raceTime;

  if (newPos > 1) {
    line.gap = values[MSG_LAP_GAP];
    if ($.isNumeric(line.gap))
      line.gap = parseFloat(line.gap).toFixed(1);
  }


  if ($scope.positions[0].num == raceNum) {
    line.int = lapNum;
    for (var j = 1; j < $scope.competitorStyle.length; ++j) {
      $scope.competitorStyle[j] = 'default';
      $scope.lapNumStyle[j] = 'default';
    }
  }

  updateGaps($scope, newPos);
  updateIntervalStyle($scope, newPos);
  updateLaptimeStyle($scope, newPos);
  
  $scope.competitorCrossedLineOnLap[raceNum] = line.lapNum;
}

function updateSectorOneStyle($scope, pos) {
  if (!pos || pos < 1) return;

  var line = $scope.positions[pos - 1];
  var competitor = $scope.competitors[line.num];

  if ($.isNumeric(line.s1)) {
    if (line.s1.trim() && !line.s2.trim() && !line.s3.trim())
      $scope.s1Style[pos - 1] = 'recent'
     else
       $scope.s1Style[pos - 1] = 'default';
       
    if (competitor.bestSectors[0] == 0 || line.s1 <= competitor.bestSectors[0]) {
      competitor.bestSectors[0] = line.s1;
      $scope.s1Style[pos - 1] = 'personalBest';
    }

    if ($scope.sessionBestSector[0] == 0 || line.s1 <= $scope.sessionBestSector[0]) {
      ClearPreviousSessionBestSector($scope, 1)
      $scope.sessionBestSector[0] = line.s1;
      $scope.previousSessionBestSectorStyle[0] = $scope.s1Style[pos - 1];
      $scope.s1Style[pos - 1] = 'sessionBest';
    }
  }
}

function durationToSecs(duration) {
  if (!duration.trim()) return;

  // Less than a minute, e.g. 34.123
  if ($.isNumeric(duration))
    return duration;

  // Over a minute, over an hour?
  var hours = 0;
  var mins = 0;
  var secs = 0;

  var colons = duration.match(/:/g).length;
  if (colons > 0) {
    var parts = duration.split(':');
    switch (colons) {
      case 2:
        hours = parts[0];
        mins = parts[1];
        secs = parts[2];
        break;

      case 1:
        mins = parts[0];
        secs = parts[1];
        break;
    }
  } else {
    return;
  }

  return parseFloat(hours) * 3600 + parseFloat(mins) * 60 + parseFloat(secs);
}

function updateLaptimeStyle($scope, pos) {
  if (!pos || pos < 1) return;

  var line = $scope.positions[pos - 1];
  var competitor = $scope.competitors[line.num];

  var secs = durationToSecs(line.lap);
  if (secs) {
    if (competitor.bestLap == 0 || secs <= competitor.bestLap) {
      $scope.lapNumStyle[pos - 1] = 'personalBest';
      competitor.bestLap = secs;
    } else {
      if (line.s3.trim())
        $scope.lapNumStyle[pos - 1] = 'recent';
      else
        $scope.lapNumStyle[pos - 1] = 'default';
    }

    if ($scope.sessionBestLap == 0 || secs <= $scope.sessionBestLap) {
      ClearPreviousSessionBestLap($scope)
      $scope.sessionBestLap = secs;
      $scope.previousSessionBestLapStyle = $scope.lapNumStyle[pos - 1];
      $scope.lapNumStyle[pos - 1] = 'sessionBest';
    }
  }
}

function ClearPreviousSessionBestLap($scope) {
  var styleArray;
  var previousStyle = 'default';

  for (var j = 0; j < $scope.lapNumStyle.length; ++j) {
    if ($scope.lapNumStyle[j] == 'sessionBest')
      $scope.lapNumStyle[j] = $scope.previousSessionBestLapStyle;
  }
}


function ClearPreviousSessionBestSector($scope, sector) {
  var styleArray;
  var previousStyle = 'default';
  switch (sector) {
    case 1:
      styleArray = $scope.s1Style;
      previousStyle = $scope.previousSessionBestSectorStyle[0];
      break;

    case 2:
      styleArray = $scope.s2Style;
      previousStyle = $scope.previousSessionBestSectorStyle[1];
      break;

    case 3:
      styleArray = $scope.s3Style;
      previousStyle = $scope.previousSessionBestSectorStyle[2];
      break;
  }

  for (var j = 0; j < styleArray.length; ++j) {
    if (styleArray[j] == 'sessionBest')
      styleArray[j] = previousStyle;
  }
}

// TODO(DS) generalize for s1..n (3)
function updateSectorTwoStyle($scope, pos) {
  if (!pos || pos < 1) return;

  var line = $scope.positions[pos - 1];
  var competitor = $scope.competitors[line.num];

  if ($.isNumeric(line.s2)) {
    if (line.s1.trim() && line.s2.trim() && !line.s3.trim())
      $scope.s2Style[pos - 1] = 'recent'
     else
       $scope.s2Style[pos - 1] = 'default';
       
    if (competitor.bestSectors[1] == 0 || line.s2 <= competitor.bestSectors[1]) {
      competitor.bestSectors[1] = line.s2;
      $scope.s2Style[pos - 1] = 'personalBest';
    }

    if ($scope.sessionBestSector[1] == 0 || line.s2 <= $scope.sessionBestSector[1]) {
      ClearPreviousSessionBestSector($scope, 2)
      $scope.sessionBestSector[1] = line.s2;
      $scope.previousSessionBestSectorStyle[1] = $scope.s2Style[pos - 1];
      $scope.s2Style[pos - 1] = 'sessionBest';
    }
  }
}

function updateSectorThreeStyle($scope, pos) {
  if (!pos || pos < 1) return;

  var line = $scope.positions[pos - 1];
  var competitor = $scope.competitors[line.num];

  if ($.isNumeric(line.s3)) {
    if (line.s1.trim() && line.s2.trim() && line.s3.trim())
      $scope.s3Style[pos - 1] = 'recent'
     else
       $scope.s3Style[pos - 1] = 'default';
       
    if (competitor.bestSectors[2] == 0 || line.s3 <= competitor.bestSectors[2]) {
      competitor.bestSectors[2] = line.s3;
      $scope.s3Style[pos - 1] = 'personalBest';
    }

    if ($scope.sessionBestSector[2] == 0 || line.s3 <= $scope.sessionBestSector[2]) {
      ClearPreviousSessionBestSector($scope, 3)
      $scope.sessionBestSector[2] = line.s3;
      $scope.previousSessionBestSectorStyle[2] = $scope.s3Style[pos - 1];
      $scope.s3Style[pos - 1] = 'sessionBest';
    }
  }
}

function updateIntervalStyle($scope, pos) {
  if (!pos || pos < 2) return;

  var line = $scope.positions[pos - 1];
  var competitor = $scope.competitors[line.num];

  if ($.isNumeric(line.int)) {
    if (competitor.lastInterval != 0 && line.int < competitor.lastInterval)
      $scope.intervalStyle[pos - 1] = 'recent';
    else
      $scope.intervalStyle[pos - 1] = 'default';
  }

  $scope.competitors[line.num].lastInterval = line.int;
}

// TODO(DS) camelCase function name.
function ProcessSectorMessage($scope, msg) {
  var MSG_SEC_RACE_NUM = 3;
  var MSG_SEC_LAP_NUM = 4;
  var MSG_SEC_SEC_NUM = 5;
  var MSG_SEC_SEC_TIME = 6;

  var values = msg.split(",");

  var raceNum = values[MSG_SEC_RACE_NUM];
  var lapNum = values[MSG_SEC_LAP_NUM];
  var secNum = values[MSG_SEC_SEC_NUM];
  var raceTime = values[Page1.MSG_RACE_TIME];

  var newPos = CheckRaceOrder($scope, raceNum, lapNum, secNum);

  if (secNum == 3) {
    $scope.competitorStyle[newPos - 1] = 'recent';
    //$scope.lapNumStyle[newPos - 1] = 'recent';
  }

  var line = $scope.positions[newPos - 1];
  
  switch (values[MSG_SEC_SEC_NUM]) {
    case '1':
      line.s1 = values[MSG_SEC_SEC_TIME];
      if ($.isNumeric(line.s1))
        line.s1 = parseFloat(line.s1).toFixed(1);
      line.s2 = '';
      line.s3 = '';
      break;
    case '2':
      line.s2 = values[MSG_SEC_SEC_TIME];
      if ($.isNumeric(line.s2))
        line.s2 = parseFloat(line.s2).toFixed(1);
      break;
    case '3':
      line.s3 = values[MSG_SEC_SEC_TIME];
      if ($.isNumeric(line.s3))
        line.s3 = parseFloat(line.s3).toFixed(1);
      $scope.competitorCrossedLineOnLap[raceNum] = line.lapNum;
      break;
  }

  updateSectorOneStyle($scope, newPos);
  updateSectorTwoStyle($scope, newPos);
  updateSectorThreeStyle($scope, newPos);
  updateLaptimeStyle($scope, newPos);

  line.lapNum = values[MSG_SEC_LAP_NUM];
  line.raceTime = raceTime;
}

function updateGaps($scope, pos) {
  if (pos == 1) return;

  var leadersLine = $scope.positions[0];  
  if (!$.isNumeric(leadersLine.int)) return;

  var thisLine = $scope.positions[pos - 1];  
  if (pos == 2) {
    thisLine.int = thisLine.gap;
    return;
  }
  
  var prevLine = $scope.positions[pos - 2];
  thisLine.int = timeDiff(thisLine.gap, prevLine.gap);
}

function timeDiff(lhs, rhs) {
  // lhs and rhs need to be in decimal for e.g. 72.01 not 1:12.01
  // TODO(ds) make the conversion on a copy and use that.

  var diff = '';
  if ($.isNumeric(lhs) && $.isNumeric(rhs))
    diff = (lhs - rhs).toFixed(1);

  return diff;
}

function ChangeRaceOrder(positions, currentPos, newPos) {
  if (currentPos > newPos)
    MoveCompetitorUp(positions, currentPos, newPos);
  else
    MoveCompetitorDown(positions, currentPos, newPos);
}

function MoveCompetitorUp(positions, currentPos, newPos) {
  if (currentPos < newPos) return;

  while (currentPos != newPos) {
    SwapLines(positions, currentPos, currentPos - 1);
    --currentPos;
  }
}

function MoveCompetitorDown(positions, currentPos, newPos) {
  if (currentPos > newPos) return;

  while (currentPos != newPos) {
    SwapLines(positions, currentPos, currentPos + 1);
    ++currentPos;
  }
}

function SwapLines(positions, lhs, rhs) {
  var ll = positions[lhs - 1];
  var rl = positions[rhs - 1];

  var tmp = new TimingLine(ll.pos, ll.num, ll.name, ll.gap, ll.int, ll.lap, ll.s1, ll.s2, ll.s3, ll.lapNum, ll.raceTime);

  ll.num = rl.num;
  ll.name = rl.name;
  ll.gap = rl.gap;
  ll.int = rl.int;
  ll.lap = rl.lap;
  ll.s1 = rl.s1;
  ll.s2 = rl.s2;
  ll.s3 = rl.s3;
  ll.lapNum = rl.lapNum;
  ll.raceTime = rl.raceTime;

  rl.num = tmp.num;
  rl.name = tmp.name;
  rl.gap = tmp.gap;
  rl.int = tmp.int;
  rl.lap = tmp.lap;
  rl.s1 = tmp.s1;
  rl.s2 = tmp.s2;
  rl.s3 = tmp.s3;
  rl.lapNum = tmp.lapNum;
  rl.raceTime = tmp.raceTime;
}

function GetCurrentPos($scope, raceNum) {
  var positions = $scope.positions;
  for (var j = 0; j < positions.length; ++j)
    if (positions[j].num == raceNum)
      return j + 1;

  return 0;
}

// TODO(DS) Lapped drivers? Backmarkers?
function GetNewPos($scope, lapNum, secNum) {
  var positions = $scope.positions;

  var lead = positions[0];
  var leadLap = lead.lapNum;
  if (lapNum > leadLap)
    return 1;

  for (var j = 0; j < positions.length; ++j) {
    var sector = positions[j].s1;

    switch (secNum) {
      case '2':
        sector = positions[j].s2;
        break;
      case '3':
        sector = positions[j].s3;
        break;
    }

    // An empty slot on the first lap?
    if (!sector.trim() && positions[j].lapNum <= lapNum)
      return j + 1;

    // Or any lower lap.
    if (positions[j].lapNum < lapNum)
      return j + 1;
  }

  return 0;
}

angular.module('raceApp', ['timer']);
function raceController($scope) {
  $scope.timerRunning = true;

  $scope.startTimer = function () {
    $scope.$broadcast('timer-start');
    $scope.timerRunning = true;
  };

  $scope.stopTimer = function () {
    $scope.$broadcast('timer-stop');
    $scope.timerRunning = false;
  };

  $scope.trackStatus = { green: 'Green', scs: 'Safety Car Standby', scd: 'Safety Car Deployed', red: 'Red' };
  $scope.sessionBestSector = [0, 0, 0];
  $scope.previousSessionBestSectorStyle = ['', '', ''];
  $scope.sessionBestLap = 0;

  $scope.positions = [new TimingLine(0, 0, '', '', '', '', '', '', '', '1', '0:00.000')];
  for (var j = 0; j < 21; ++j)
    $scope.positions[$scope.positions.length] = new TimingLine(0, 0, '', '', '', '', '', '', '', '1', '0:00.000');

  $scope.competitors = new Object();

  $scope.competitorCrossedLineOnLap = new Object();

  // Create the style arrays to dynamically colour the grid.
  $scope.competitorStyle = ['recent'];
  for (j = 0; j < 21; ++j)
    $scope.competitorStyle[$scope.competitorStyle.length] = 'recent';

  $scope.lapNumStyle = ['recent'];
  for (j = 0; j < 21; ++j)
    $scope.lapNumStyle[$scope.lapNumStyle.length] = 'recent';

  $scope.intervalStyle = ['default'];
  for (j = 0; j < 21; ++j)
    $scope.intervalStyle[$scope.intervalStyle.length] = 'default';

  $scope.s1Style = ['default'];
  for (j = 0; j < 21; ++j)
    $scope.s1Style[$scope.s1Style.length] = 'default';

  $scope.s2Style = ['default'];
  for (j = 0; j < 21; ++j)
    $scope.s2Style[$scope.s2Style.length] = 'default';

  $scope.s3Style = ['default'];
  for (j = 0; j < 21; ++j)
    $scope.s3Style[$scope.s3Style.length] = 'default';
}

raceController.$inject = ['$scope'];

function TimingLine(pos, num, name, gap, int, lap, s1, s2, s3, lapNum, raceTime) {
  this.pos = pos;
  this.num = num;
  this.name = name;
  this.gap = gap;
  this.int = int;
  this.lap = lap;
  this.s1 = s1;
  this.s2 = s2;
  this.s3 = s3;
  this.lapNum = lapNum;
  this.raceTime = raceTime;
}

function Competitor(raceNum, gridNum, shortName, name, team, bestSectors, bestLap, lastInterval) {
  this.raceNum = raceNum;
  this.gridNum = gridNum;
  this.shortName = shortName;
  this.name = name;
  this.team = team;
  this.bestSectors = bestSectors;
  this.bestLap = bestLap;
  this.lastInterval = lastInterval;
}
