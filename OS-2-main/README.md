# OS-2

## Part A
In this part, we create the server that supports the given protocol.


## Part B
In this part, When we download a file whose extension is .list, it has a list of files.
We want to download all the files at the same time.
To support simultaneous downloading, we will open several connections to the same server simultaneously (like the download accelerator program) and receive data in all of them. We will support receiving data on several connections simultaneously with the help of async i/o.

## Running the code:

To run the code you need first to put all the files you want to download in a folder in the main folder and call it files.
Also, the files that we want to download using the get list need to be in this folder.
So now we will have those three folders: </br >
<img width="200" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/ebc05b42-78dc-41dc-b1ea-70754d7907dc">
<br />
<br />
We created a make file recursively that will build all the files in one run, to run the make file recursively you need to run: `make all` in the main loop. <br />
<img width="500" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/77ae368c-d0b2-4413-b7af-b4f9428a64f4">
<br />
We also need to create a folder called `files` in the client folder. In this folder, all the files that we get from the server are saved. <br />
<img width="123" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/9addf211-7386-460f-a935-06012e88d834">

### Running the server:
To run the server, we need to enter the folder server first from the main directory: `cd server` <br />
<img width="225" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/18aeaa8f-4619-4124-936c-e2ef8e5984c6"> <br />
Then, we will run the server: <br />
The first argument needs to be the server itself, meaning: `./server` and the second argument needs to be the folder in the server directory that we want to save all the products from the POST in. We choose `files`. <br />
<img width="267" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/b89dd413-9ec4-47be-835c-7ef91f1fd85a"> <br />
So Now we run the server and create a folder called files in the server folder. <br />
Our server waits for a connection from the client. 

### Running the client:
To run the client, we need to enter the folder client first from the main directory: `cd client` <br />
<img width="225" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/1367fce4-64c5-46ea-83d7-24a521d35fff"> <br />
Then, we will run the client: <br />
The first argument needs to be the client itself, meaning: `./clientb` and the second argument needs to be the hostname of the owner of the ubuntu. To find the owner we will run the following command: `hostname`. <br />
<img width="267" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/47975715-4638-49e7-9953-f4b583076eb1">
So now we can see that this Ubuntu hostname is: "moria".
So Now we run the client: <br />
<img width="269" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/fabd28c9-9ef2-4111-8a62-96b64bcf7eb5">
<br /><br />
When we run the client we need to send to the server some commands: GET or POST. Those commands we get from the user when we run the client. <br />
<img width="271" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/2025394d-aa75-4166-a867-ddc53bf30c06"> <br />
As we can see, the client is waiting for input from the user.<br />
## We will show you some commands that the client can send to the user:
### POST
In this command, the client sends to the server contents, and the server saves these contents in the folder that we created:
The command needs to look like this:<br /> `POST <REMOTEPATH> <CRLF>` <br />
`<CONTENTS> <CRLF>` <br />
`<CRLF> `
<br />
![image](https://github.com/moriagr/OS-2/assets/99357654/837a3af2-8e60-4ef1-aba0-84c40c81529b) <br />
So in this command, we send the file name where we want to save the contents, and the contents itself.<br />
<img width="246" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/3b3946d0-4133-4ea5-9b1f-f34c9a7af46f"> <br />
As we can see, this command created a new file in the files folder in the server folder with the contents we sent to.

### GET
In this command, the client sends to the server the name of the file that we want to download for our client, and the server sends the decoded content of this file to the client so the client saves the contents of the file in the folder that we created:
**The files that we want to get needs to be in the main folder in the files directory. It should look like this:** `../files/*.*`

The command needs to look like this:<br /> `GET <REMOTEPATH> <CRLF>` <br />
`<CRLF> `
<br />
This is how the client side looks like: <br />
<img width="500" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/a38fc9d8-b011-411b-bea7-af5cdce3248c">
<br />
This is how the server-side looks like: <br /> <img width="332" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/a6afff73-7856-49fe-ae5e-8b52d7456cdf">
<br />
<img width="110" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/084f5f07-3230-4385-ab2f-b24bbead4dbe">
 <br />
As we can see, this command created a new file in the files folder in the client folder with the contents from the file we asked for from the server.

### GET List
In this command, the client sends to the server the name of the file list that we want to download for our client.
The file list contains a list of all the files we want to download from the server. The server returns the contents of the file, in this case, it's the list of all the files that we want to download, in each row there is a new file to download.<br />
For each file, the client asks the server to send the decoded content of this file to the client, so the client saves the contents of the file in the folder that we created:
The command needs to look like this:<br /> `GET <REMOTEPATH>.list <CRLF>` <br />
`<CRLF> `
<br />
This is how the client side looks like: <br />
<img width="338" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/fdd6bf39-d7ae-4a04-8710-c2d5f1b679e5">
<br />
<img width="241" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/d1c77bd1-0cee-4e68-a44a-8538a0e52378">
<br />
This is how the server-side looks like: <br /> <img width="332" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/fee15abd-c452-45fa-9ba0-5748cb92dc0a">
<br />
<img width="317" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/d9fbc8ae-08dd-45e0-9cbc-98b204f25d82">

<br />
<img width="317" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/084f5f07-3230-4385-ab2f-b24bbead4dbe">
 <br />
This is the file list that contains all the files name: <br />
<img width="151" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/1404a94d-0870-4b94-844f-b6398e4fdfba">
<br />
Now, those are the files we created: <br />
<img width="103" alt="image" src="https://github.com/moriagr/OS-2/assets/99357654/dcfeda5d-cc45-44bb-a0bb-295d20d50cb1">
