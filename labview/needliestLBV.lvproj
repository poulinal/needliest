<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="25008000">
	<Property Name="NI.LV.All.SaveVersion" Type="Str">25.0</Property>
	<Property Name="NI.LV.All.SourceOnly" Type="Bool">true</Property>
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="ArduinoController.lvclass" Type="LVClass" URL="../ArduinoController/ArduinoController.lvclass"/>
		<Item Name="LabjacksController.lvclass" Type="LVClass" URL="../LabjacksController/LabjacksController.lvclass"/>
		<Item Name="modesEnum.ctl" Type="VI" URL="../modesEnum.ctl"/>
		<Item Name="Needle Valve labVIEW Code.vi" Type="VI" URL="../Needle Valve labVIEW Code.vi"/>
		<Item Name="PIDController.lvclass" Type="LVClass" URL="../PIDController/PIDController.lvclass"/>
		<Item Name="Dependencies" Type="Dependencies"/>
		<Item Name="Build Specifications" Type="Build"/>
	</Item>
</Project>
