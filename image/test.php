<style>
  .content_0 {width:1000px; overflow: hidden; display:none; word-wrap: break-word; word-break: break-all;}
  .content_1 {width:1000px; overflow: hidden; display:block; word-wrap: break-word; word-break: break-all;}
  .head_0 {width:1000px; border: 1px solid gray; background-color: white;}
</style>
<script>
  function show_content(e) {
    var content = e.nextSibling;
    if("content_0" == content.getAttribute("class")) {
      content.setAttribute("class", "content_1");
    } else {
      content.setAttribute("class", "content_0");
    }
  }
</script>
<?php
  $pdo = new PDO("sqlite:/usr/share/data.db");
  $stmt = $pdo->prepare("select * from data order by id DESC limit 0, 30;");
  $stmt->execute();
  $rows = $stmt->fetchAll();
  foreach($rows as $row) {
    $data = gzuncompress($row["data"]);
    $h_data = unpack("H*", $data);
    echo '<div class="head_0" onclick="show_content(this)">'.$row["id"]." ".$row["time"]." ".strlen($data).'</div>';
    echo '<div class="content_0">'.$h_data[1].'</div>';
  }
  unset($row);
  $pdo = null;
?>