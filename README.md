<!DOCTYPE html>
<html>
<head>
<h1> Testing-system </h1>
</head>
<body>
<p> Testing System is a type of software testing that is performed on a complete integrated system to evaluate the compliance of the system with the corresponding requirements.

In system testing, integration testing passed components are taken as input. The goal of integration testing is to detect any irregularity between the units that are integrated together. System testing detects defects within both the integrated units and the whole system. The result of system testing is the observed behavior of a component or a system when it is tested 

Result of checking will be written in terminal. </p>
  <h2> Testing system supports:</h2>
  <ol>
  <li> checking multiple tests </li>
  <li> custom time limit to complete task </li>
  <li> two types of checkers: integer and byte </li>
  <li> configs </li>
  <li> logs </li>
  </ol>
  <h2> Testing system stucture </h2>
  <p2> 
   sources contains code for judge
  
   contest - contestants folder, tests and correct answers to them, config files
       
   tmp - temporary files like contestants binary files 
       
   bin - judge binary file
   </p2>
  <h2> Prepare testing system </h2>
  <ol>
  <li> put contestants tests in code folder </li>
  <li> put all tests in tests folder </li>
  <li> fill global.cfg and checker.cfg with data </li>
  </ol>
  <h2> Prepare global.cfg </h2>
  <p> Global.cfg contains information about time to complete the task, number of tests to check. All statements should end with ';' </p>
  <ol>
  <li>  Write after "task_time =" - write number in seconds to set time to complete tasks. E.g. task_time = 10; </li>
  <li>  Write after "task's test count:" - write Task name and count of tests. E.g. A = 3; </li>
  </ol>
  <h2> Prepare checker.cfg </h2>
  <p2> Checker.cfg contains information about type of checking.
  Write after "check type:" - write Task name and type of checker. E.g. A = i; </p2>
  <h2> logs </h2>
  <p2>  a log file is a file that records either events that occur in software run.</p2>
  <p2>
  Logs contain result of checking, errors etc. In our case, messages are written to a single log file. </p2>
  <h2> Prepare tests </h2>
  <p2> In task folder create two types of files: .dat and .ans.
  
  .dat - task
  
  .ans - answer to task
  </p2>
  <h1> DISCLAIMER: Task's folder name and contestant's task name sould be the same to work properly </h1>
  <h2> How to compile and run program: </h2>
  <ol> 
  <li> open terminal in directory </li>
  <li> run in terminal: make (it will save binary files in bin directory) </li>
  <li> run in terminal: ./bin/judge <li>
  </ol>
  <h2> to clean tmp folder and binary files </h2>
  <p2> run in terminal: make clean </p2>
</body>
</html>
