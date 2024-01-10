/** Librerías para conexión circuito-servidor. */
// NOTA: El servidor es la parte con la que realizamos el envío y recepción de datos, tanto del lado del circuito,
// como en el lado de la interfaz gráfica.
const { SerialPort } = require("serialport"); /** Se llama al módulo que se
utiliza para la comunicación con el puerto serial. */
const { ReadlineParser } = require("@serialport/parser-readline");

/** Librerías para comunicación servidor-interfaz */
const express = require("express");
const http = require("http");
const path = require("path");
const morgan = require("morgan");
const { Server } = require("socket.io");
const cors = require("cors");

/** Configuración del puerto serial. */
// NOTA: En el lado del servidor no es necesario establecer el protocolo RS232, pues viene por
// al trabajar con el módulo de seriaport por defecto.
const port = new SerialPort({
  path: "COM4",
  baudRate: 9600,
}); /** Se crea el puerto donde se llevará la comunicación mediante un objeto.
- "COM4": nombre del puerto serial.
- {baudRate: 9600}: atributo del objeto SerialPort.
*/

const parser = port.pipe(new ReadlineParser());
/** Permite leer lo que se recibe desde el puerto serial y lo transforma a caracteres. */

/** Configuración del servidor */
const app = express();
app.set("port", process.env.PORT || 4000);
/** Configuración de Middlewares */
app.use(cors());
app.use(morgan("dev"));
// app.use(express.json());

/** Configuración de protocolo HTTP para conexión de interfaz con la red. */
const server = http.createServer(app);

/** Configuración del 'socket' que realiza la comunicación de los datos obtenidos por el puerto serial hacía el servidor. */
// NOTA: Un socket se trata de un canal de comunicación que permite el intercambio
// de información entre programas de distinto uso y entre redes.
// const io = new SocketSever(server);
const io = new Server(server, {
  cors: {
    origin: "http://localhost:3000",
  },
});

/** Rutas */
/* app.get("/", (req, res) => {
  res.sendFile("../tests/index.html");
}); */

io.on("connection", (socket) => {
  console.log("Envío de datos por el socket " + socket.id);
  console.log("Interfaz de " + socket.id + " conectada.\n");

  socket.on("sendChar", (char) => {
    console.log("Opción seleccionada: " + char);
    port.write(char);
  });

  socket.on("disconnect", () => {
    console.log("Interfaz de " + socket.id + " desconectada.\n");
  });
});

parser.on("data", (line) => {
  console.log("Microcontrolador conectado...\n");
  // console.log(line.toString());
  const data = line.toString();
  io.emit("dataFromPIC", data);
}); /** Recibe los datos del microcontrolador y los envía a la interfaz correspondiente mediante el servidor. */

parser.on("error", (err) => {
  console.log("Error en comunicación: ", err);
}); /** Notifica la existencia de un error en el envío de los datos del PIC al programa de servidor. */

/** Inicializa el servidor mediante protocolo HTTP. */
// NOTA: 'escuchar' se utiliza para hablar acerca de la apertura de una fuente de datos de un programa
// en un puerto específico, en el contexto de desarrollo Web.
server.listen(app.get("port"), () => {
  console.log("Servidor escuchando en el puerto " + app.get("port"));
});
