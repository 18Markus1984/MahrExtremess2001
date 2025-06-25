#include <Arduino.h>

//Raw content of index.html in plain text
static const char *index_html = R"--espform--(
<!DOCTYPE html>
<html lang="en">

<head>
  <title> Simple Textbox </title>
  <style>
body {
    padding: 20px;
  }
  
  label {
    font-size: 17px;
    font-family: sans-serif;
  }
  
  input {
    display: block;
    width: 300px;
    height: 40px;
    padding: 4px 10px;
    margin: 10 0 10 0;
    border: 1px solid #03A9F4;
    background: #cce6ff;
    color: #1c87c9;
    font-size: 17px;
  }
}
  </style>
</head>

<body>
  <form>
    <label for="text1">Value to device</label>
    <input type="text" id="text1" name="text1" value="Change me..."/>
    <label for="text2">Value from device</label>
    <input type="text" id="text2" name="text2" /> </form>
</body>

</html>
)--espform--";
