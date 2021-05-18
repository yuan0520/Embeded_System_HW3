# Embedded System HW3

## (1) how to setup and run my program

### 1. Variable declaration

以下介紹幾個重要的變數，`mode` 這個變數控制模式的切換，他會在`RPC_LOOP`、`GESTURE_UI_MODE`、`TILT_ANGLE_DETECTION_MODE` 這三個變數間切換，當 `mode` 的值等於對應到的模式時，才會在該模式中進行資料搜集、判斷等動作。

![](https://i.imgur.com/rrMLMxO.png)

`angle`、`angle_sel`、`angle_det` 這三個變數則分別代表透過手勢偵測到的角度、按下 user button 時選定的角度，以及在 tilt angle detection mode 中偵測到的傾角。

![](https://i.imgur.com/TRIAN5C.png)



### 2. Two custom RPC functions

### (1) Gesture UI mode

首先，我先定義了屬於 Gesture UI 的 RPC function `gesture`，且若在 screen 中輸入 `/gesture/run` 時便可以呼叫此 RPC function。

![](https://i.imgur.com/ZPUeYW1.png)

而 gesture UI mode 也有他對應的 thread 和 event queue，當此 RPC function 被呼叫時，並會呼叫我定義的 function `gesture_UI_mode`，並在 `gesture_t` 這個 thread 中執行。

![](https://i.imgur.com/2sJXYSm.png)

而 `gesture_UI_mode` 這個 function 也是主要用來控制這個模式的 function，接下來來介紹他的運作方式：

首先，因為題目要求進入此模式時要透過 LED 燈的閃爍來示意，所以我將 `blink_time` 這個變數初始值設為 `10`，所以 LED 燈總共會有 5 個明暗的來回，其餘有關 LED 燈閃爍的實作都和這裡一樣。

![](https://i.imgur.com/BjDwMe0.png)

![](https://i.imgur.com/VmpodRZ.png)

接下來我利用 Lab 8 實作過關於判斷手勢的 code，在我畫圖案的時候，便可以得到 `gesture_index` 這個變數的值，來看看我畫的是哪種手勢。

![](https://i.imgur.com/0rxXJE0.png)

因為有三種手勢的選項，而三種手勢我分別將他們對應到 `15` 度、`30` 度以及 `45` 度，當作等等 tilt angle detection mode 的參考角度。當確認是哪個手勢以及對應的角度之後，也會呼叫 `menu` 這個函式去更改 uLCD 的顯示，並丟到跟控制 menu 有關的 event queue 和他對應的 thread 去執行。

![](https://i.imgur.com/U0MWF6U.png)

![](https://i.imgur.com/pRFqWl4.png)

在選擇角度之後，我們要透過按下 user button 來將選定的角度 publish 給 MQTT broker，所以我在 `main` function 中先設定好 user button 這個 interrupt signal，在 user button 被按下去後，便會呼叫對應的 function 去做 publish 的動作，而 wifi/MQTT 部分的 code 和 lab 中幾乎都一樣，這裡我就不附上了。

![](https://i.imgur.com/fuGGXCy.png)

當按下 user button 呼叫 publish 的函式時，便會將選定角度的訊息 publish 出去並在 python 端 (mqtt_client)顯示訊息，同時也會更改 uLCD 的顯示，以及切換回 `RPC_LOOP` 的模式。以上便是 `Gesture UI mode` 的運作。

![](https://i.imgur.com/hdL9VAK.png)



### (2) Tilt angle detection mode

首先，我也先替這個模式定義了對應的 RPC function，所以在 screen 中輸入 `/angle_d/run` 時便可以呼叫這個 RPC function。

![](https://i.imgur.com/2hupySc.png)

控制 thread、event queue 以及 LED 燈的方式都和 Gesture UI mode 一樣，所以在這裡我就跳過前面的部分。

在此模式中，我會先量一個參考的三軸加速器座標，以利後續傾角的計算。當量完之後會顯示出參考座標，並閃爍 LED 燈。

![](https://i.imgur.com/CgJFqSr.png)

接下來就是計算傾角的主要部分，但首先可以注意到 while 迴圈的判斷依據就是 `mode` 的值，在其他模式中也是這樣判斷的，以確保其他模式的程式該被終止卻沒被終止。

其實函式裡面也就是利用數學概念計算傾角，並 show 在 uLCD 上，然後如果超過先前選定的傾角就 publish 出去。

![](https://i.imgur.com/tGBeqgQ.png)

### 3. About sending comments (mqtt_client.py)

首先我先定義了兩個 RPC function `back` 和 `back_finished`，分別是按下 user button 要回到 RPC loop 時會被呼叫，第二個是當傾角超過選定的角度被 publish 十次後要終止 tilt angle detection mode 回到 RPC loop 時被呼叫。

![](https://i.imgur.com/peyrRAT.png)

接下來是跟傳送指令和 publish message 也很有關係的 `mqtt_client.py`，首先我定義了兩個 topic，`angle_sel` 這個 topic 對應到按下 user button 時 publish 出來的訊息，而 `angle_det` 這個 topic 對應到偵測傾角時 publish 出來的訊息。

![](https://i.imgur.com/3tOrS9f.png)

所以當收到訊息時，我會先判斷是哪個 topic，若是 `angle_sel`，則印出選擇的角度，並傳回 `/back/run` 指令呼叫 `back` 這個 RPC function，讓程式結束 gesture UI mode 回到 RPC loop。

若 topic 為 `angle_det`，則我除了印出訊息外，還會判斷現在 publish 了第幾次，若已經十次了，就傳回 `/back_finished/run` 指令呼叫 `back_finished` 這個 RPC function，讓程式結束 tilt angle detection mode 回到 RPC loop。

![](https://i.imgur.com/7caBcSd.png)


---

## (2) what are the results

### 1. uLCD display

#### (1) 選擇角度時的畫面

<img src="https://i.imgur.com/iNrGdGN.jpg" width="50%" height="50%" />

#### (2) 角度選定後顯示選定的角度

<img src="https://i.imgur.com/sOhK1te.jpg" width="50%" height="50%" />


#### (3) 偵測角度時即時更新傾角

<img src="https://i.imgur.com/dBINPXJ.jpg" width="50%" height="50%" />

---

### 2. Screen display

#### (1) 選定角度過程

首先輸入 `/gesture/run` 進入 gesture UI mode，選定角度後按下 user button，便會顯示我們選定的角度，並將訊息 publish 給 python client 端。
 
![](https://i.imgur.com/J7nJLaF.jpg)

#### (2) 選定參考座標

輸入 `/angle_d/run` 指令後會進入 tilt angle detection mode，量到參考座標後會顯示出來，並開始測量現在的座標和計算傾角。

![](https://i.imgur.com/iEFZt4b.jpg)

#### (3) 計算傾角、publish message

當目前板子的傾角超過我們選定的角度時便會將偵測到的角度 publish 到 python client 端，且在 publish 超過十次之後由 python client 端送出 `/back_finished/run` 指令，退出 tilt angle detection mode 回到 RPC loop。

![](https://i.imgur.com/9Oifync.jpg)
