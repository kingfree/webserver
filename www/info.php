<!DOCTYPE html>
<html>
    <head>
        <title>
            信息
        </title>
    </head>
    <body>
        <h1>PHP 解析测试</h1>
<?php

echo "GET的数据:<pre>";
print_r($_GET);
echo "</pre>\n";

echo "POST的数据:<pre>";
print_r($_POST);
echo "</pre>\n";

for ($i = 0; $i < 10; $i++) {
    echo "<p>$i</p>\n";
}
?>
    </body>
</html>

