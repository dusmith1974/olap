"use strict";

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

        $scope.competitors[values[COM_GRID_POS] - 1] = new Competitor(values[COM_RACE_NUM], values[COM_GRID_POS], values[COM_SHORT_NAME], values[COM_NAME], values[COM_TEAM_NAME]);
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

  var newPos = CheckRaceOrder($scope, raceNum, lapNum, '3');
  var line = $scope.positions[newPos - 1];

  $scope.competitorStyle[newPos - 1] = 'recent';
  $scope.lapNumStyle[newPos - 1] = 'recent';

  if (newPos > 1)
    line.gap = values[MSG_LAP_GAP];

  line.lap = values[MSG_LAP_TIME];

  if ($scope.positions[0].num == raceNum) {
    line.int = lapNum;
    for (var j = 1; j < $scope.competitorStyle.length; ++j)
      $scope.competitorStyle[j] = 'default';

     for (j = 1; j < $scope.lapNumStyle.length; ++j)
      $scope.competitorStyle[j] = 'default';
  }

  $scope.competitorCrossedLineOnLap[raceNum] = line.lapNum;
}


function ProcessSectorMessage($scope, msg) {
  var MSG_SEC_RACE_NUM = 3;
  var MSG_SEC_LAP_NUM = 4;
  var MSG_SEC_SEC_NUM = 5;
  var MSG_SEC_SEC_TIME = 6;

  var values = msg.split(",");

  var raceNum = values[MSG_SEC_RACE_NUM];
  var lapNum = values[MSG_SEC_LAP_NUM];
  var secNum = values[MSG_SEC_SEC_NUM];

  var newPos = CheckRaceOrder($scope, raceNum, lapNum, secNum);

  $scope.competitorStyle[newPos - 1] = 'recent';
  $scope.lapNumStyle[newPos - 1] = 'recent';

  var line = $scope.positions[newPos - 1];
  
  switch (values[MSG_SEC_SEC_NUM]) {
    case '1':
      line.s1 = values[MSG_SEC_SEC_TIME];
      $scope.s1Style[newPos - 1] = 'recent';
      line.s2 = '';
      $scope.s2Style[newPos - 1] = 'default';
      line.s3 = '';
      $scope.s3Style[newPos - 1] = 'default';
      break;
    case '2':
      line.s2 = values[MSG_SEC_SEC_TIME];
      $scope.s2Style[newPos - 1] = 'recent';
      $scope.s1Style[newPos - 1] = 'default';
      $scope.s3Style[newPos - 1] = 'default';
      break;
    case '3':
      line.s3 = values[MSG_SEC_SEC_TIME];
      $scope.s3Style[newPos - 1] = 'recent';
      $scope.s1Style[newPos - 1] = 'default';
      $scope.s2Style[newPos - 1] = 'default';
      
      $scope.competitorCrossedLineOnLap[raceNum] = line.lapNum;
      break;
  }

  line.lapNum = values[MSG_SEC_LAP_NUM];
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

  var tmp = new TimingLine(ll.pos, ll.num, ll.name, ll.gap, ll.int, ll.lap, ll.s1, ll.s2, ll.s3, ll.lapNum);

  ll.num = rl.num;
  ll.name = rl.name;
  ll.gap = rl.gap;
  ll.int = rl.int;
  ll.lap = rl.lap;
  ll.s1 = rl.s1;
  ll.s2 = rl.s2;
  ll.s3 = rl.s3;
  ll.lapNum = rl.lapNum;

  rl.num = tmp.num;
  rl.name = tmp.name;
  rl.gap = tmp.gap;
  rl.int = tmp.int;
  rl.lap = tmp.lap;
  rl.s1 = tmp.s1;
  rl.s2 = tmp.s2;
  rl.s3 = tmp.s3;
  rl.lapNum = tmp.lapNum;
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
    if (!sector.trim() && positions[j].lapNum == 1)
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

  $scope.positions = [new TimingLine(0, 0, '', '', '', '', '', '', '', '1')];
  for (var j = 0; j < 21; ++j)
    $scope.positions[$scope.positions.length] = new TimingLine(0, 0, '', '', '', '', '', '', '', '1');

  $scope.competitors = [new Competitor(0, 0, '', '', '')];
  for (j = 0; j < 21; ++j)
    $scope.competitors[$scope.competitors.length] = new Competitor(0, 0, '', '', '');

  $scope.competitorCrossedLineOnLap = new Object();

  // Create the style arrays to dynamically colour the grid.
  $scope.competitorStyle = ['recent'];
  for (j = 0; j < 21; ++j)
    $scope.competitorStyle[$scope.competitorStyle.length] = 'recent';

  $scope.lapNumStyle = ['recent'];
  for (j = 0; j < 21; ++j)
    $scope.lapNumStyle[$scope.lapNumStyle.length] = 'recent';

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

function TimingLine(pos, num, name, gap, int, lap, s1, s2, s3, lapNum) {
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
}

function Competitor(raceNum, gridNum, shortName, name, team) {
  this.raceNum = raceNum;
  this.gridNum = gridNum;
  this.shortName = shortName;
  this.name = name;
  this.team = team;
}
