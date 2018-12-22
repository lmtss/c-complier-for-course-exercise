const { app, BrowserWindow, Menu, MenuItem } = require('electron')


let win
var menu = null;

function createWindow () {
  win = new BrowserWindow({ width: 1800, height: 900 })

  win.loadFile('index.html')

  //win.webContents.openDevTools()
  Menu.setApplicationMenu(null);
  menu = new Menu();
  
  var menuItem = new MenuItem({
    role : "toggledevtools"
  });
  menu.append(menuItem);
  Menu.setApplicationMenu(menu);

  win.on('closed', () => {

    win = null
  })
}
app.on('ready', createWindow)


app.on('window-all-closed', () => {

  if (process.platform !== 'darwin') {
    app.quit()
  }
})

app.on('activate', () => {

  if (win === null) {
    createWindow()
  }
})

