# TagSense  Plugin

TagSense is a service that allows for external processing of aircraft tags in the `VATSIM` network. This `Euroscope plugin` interfaces with the [TagSense API](https://gitlab.com/portugal-vacc/tagsense-api) at the location specified by each vACC. 

**If you would like to implement TagSense on your vACC, please contact me through my e-mail address vicente.rendo@gmail.com**

---

## 📲 Instalation
Download the latest release from the [releases page](https://github.com/vicenterendo/TagSense/releases) and set the config file according to the instructions bellow. If you are downloading for use in a vACC, consider asking for a configuration file. **If you would like to implement TagSense on your vACC, please contact me through my e-mail address vicente.rendo@gmail.com**

---

## 🔧 Config File
The configuration file allows the user to customize the plugin's behavior to their needs and is usually provided by each vACC. It can be located at the following location.
<pre> %execlocation%/TagSenseConfig.txt </pre>

- `REFRESH` Frequency at which to send the updates to the server

- `SERVER` Address of the server 

- `PREFIX` Start of the icao codes of the flights to be uploaded <sub>( LP = LPxx | E = Exxx )</sub>

---

## 💻 Commands
Commands are a way to **temporarily** change the plugin's behavior. These changes are reset everytime the plugin is reloaded. All commands start with
<pre>.tagsense (...) </pre>
- `stop` Stop sending updates to the server.
  
- `start` Start sending updates to the server.
  
- `server <addr>` Set the server address.
  
- `reload` Reload the settings from the config file.

---
## Alpha Phase
The TagSense plugin is currently in the alpha phase, meaning there is a chance that Euroscope ( being Euroscope ) will completely wet it's pants. It is currently being tested at Portugal vACC. If you would like to take part in the tests, please join the [Portugal vACC Discord Server](https://discord.portugal-vacc.org) for instructions.

