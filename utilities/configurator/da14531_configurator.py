import tkinter as tk
import tkinter.ttk as ttk
import os
import enum
import binascii
import re

from tkinter import filedialog
from config_file import Config
from config_definition import ConfigDefinition
from struct import *
from tkinter import messagebox
import common as common


class InputType(enum.Enum):
    ENTRY = 1
    COMBO = 2
    CUSTOM = 3


DEFAULT_CFG_FILE_NAME = "default.img"
DEFAULT_ICON_PHOTO = "Dialog_D-Symbol_Color_RGB_Small_icon.ico"

PORT_MAX_VALUE = 9
PIN_MAX_VALUE = 63


def is_integer_type(type_name):
    switcher = {
        "uint8_t": True,
        "int8_t": True,
        "uint16_t": True,
        "int16_t": True,
        "uint32_t": True,
        "int32_t": True,
    }
    return switcher.get(type_name, False)


def is_array_type(type_name):
    switcher = {
        "array": True,
    }
    return switcher.get(type_name, False)


def is_bd_address_type(type_name):
    switcher = {
        "bd_address_t": True,
    }
    return switcher.get(type_name, False)


def is_gpio_pin_type(type_name):
    switcher = {
        "gpio_pin_t": True,
    }
    return switcher.get(type_name, False)


class Application:
    def __init__(self):
        self.config = Config()
        try:
            self.cfg_def = ConfigDefinition()
        except Exception as inst:
            messagebox.showerror("Error", str(inst.args[0]) + ": " + "config-spec.xml " + str(inst.args[1]))
            return

        self.cfg_file_name = DEFAULT_CFG_FILE_NAME
        self.cfg_full_file_name = DEFAULT_CFG_FILE_NAME
        self.init_window()

    def set_window_position(self, window, width=300, height=200):
        # set window position to the center of the screen

        # get screen width and height
        screen_width = window.winfo_screenwidth()
        screen_height = window.winfo_screenheight()

        # calculate position x and y coordinates
        x = (screen_width / 2) - (width / 2)
        y = (screen_height / 2) - (height / 2)
        window.geometry('%dx%d+%d+%d' % (width, height, x, y))

    def update_window_title(self, cfg_name):
        self.window.title("Configurator - %s" % cfg_name)

    def update_window_icon(self, filename=DEFAULT_ICON_PHOTO):
        icon_path = common.resource_path(filename) + "\\" + filename
        self.window.iconbitmap(icon_path)

    def init_window(self):
        self.window = tk.Tk()

        # set window position at the center of the screen
        self.set_window_position(self.window, 1100, 700)

        # left and right frame adopt the window size according to geometry
        self.window.rowconfigure(0, weight=1)
        self.window.columnconfigure(1, weight=3)

        # since window is resizable, set minimum size
        self.window.minsize(width="1100", height="350")

        self.update_window_title(self.cfg_file_name)
        self.update_window_icon()

        self.init_menu()

        self.leftFrame = tk.Frame(self.window, borderwidth=2, relief=tk.GROOVE)
        self.leftFrame.grid(row=0, column=0, padx=5, pady=5, sticky=tk.NSEW)

        self.rightFrame = tk.Frame(self.window, borderwidth=2, relief=tk.GROOVE)
        self.rightFrame.grid(row=0, column=1, padx=5, pady=5, sticky=tk.NSEW)
        self.rightFrame.rowconfigure(4, weight=1)
        self.rightFrame.columnconfigure(3, weight=1)

        # --------------------------------------------------------------------------------------------------------------
        # ------------------------ left Frame -------------------------------------------------------------------------
        # --------------------------------------------------------------------------------------------------------------

        self.labelNewElement = tk.Label(self.leftFrame, text="New Element")
        self.labelNewElement.grid(row=0, column=0, sticky=tk.W, padx=5, pady=5)

        self.labelGroupID = tk.Label(self.leftFrame, text="Group ID")
        self.labelGroupID.grid(row=1, column=0, sticky=tk.E, padx=5, pady=5)

        self.groupName = tk.StringVar()
        self.groupDict = self.cfg_def.get_groups()
        self.comboboxGroupNames = ttk.Combobox(self.leftFrame, textvariable=self.groupName, width=28)
        self.comboboxGroupNames.grid(row=1, column=1, sticky=tk.W, padx=5, pady=5)
        self.comboboxGroupNames['values'] = list(self.groupDict.keys())
        self.comboboxGroupNames.current(0)
        self.comboboxGroupNames.bind("<<ComboboxSelected>>", self.on_group_changed)

        self.labelElementID = tk.Label(self.leftFrame, text="Element ID")
        self.labelElementID.grid(row=2, column=0, sticky=tk.E, padx=5, pady=5)

        self.elementName = tk.StringVar()
        self.elementDict = self.cfg_def.get_group_elements(self.groupName.get())
        self.comboboxElementName = ttk.Combobox(self.leftFrame, textvariable=self.elementName, width=28)
        self.comboboxElementName.grid(row=2, column=1, sticky=tk.W, padx=5, pady=5)
        self.comboboxElementName['values'] = list(self.elementDict.keys())
        self.comboboxElementName.current(0)
        self.comboboxElementName.bind("<<ComboboxSelected>>", self.on_element_changed)

        self.labelType = tk.Label(self.leftFrame, text="Type")
        self.labelType.grid(row=3, column=0, sticky=tk.E, padx=5, pady=5)

        self.elementID = self.elementDict[self.elementName.get()]
        self.labelTypeName = tk.Label(self.leftFrame, text=self.cfg_def.get_type_name(self.groupName.get(), self.elementID))
        self.labelTypeName.grid(row=3, column=1, sticky=tk.W, padx=5, pady=5)

        # value label and entry/combobox
        self.labelValue = tk.Label(self.leftFrame, text="Value")
        self.labelValue.grid(row=4, column=0, sticky=tk.E, padx=5, pady=5)
        self.labelPortValue = None
        self.labelPinValue = None

        self.comboboxValue = ttk.Combobox()
        self.comboboxValue1 = ttk.Combobox()
        self.comboboxValue1.visible = False
        self.entryValue = tk.Entry()
        self.valuesDict = dict()
        self.elementValue = tk.StringVar()
        self.pinValue = tk.StringVar()
        self.value_input_widget_init()

        self.labelUnits = tk.Label(self.leftFrame, text=self.cfg_def.get_units(self.groupName.get(), self.elementID))
        self.labelUnits.grid(row=4, column=1, sticky=tk.E)

        self.buttonAdd = tk.Button(self.leftFrame, text="Add", command=self.add_element_callback)
        self.buttonAdd.grid(row=5, column=1, sticky=tk.W, padx=5, pady=5)

        # --------------------------------------------------------------------------------------------------------------
        # ------------------------ right Frame -------------------------------------------------------------------------
        # --------------------------------------------------------------------------------------------------------------

        self.labelConfigurationStructure = tk.Label(self.rightFrame, text="Configuration Structure")
        self.labelConfigurationStructure.grid(row=0, column=0, sticky=tk.W)

        self.labelVersion = tk.Label(self.rightFrame, text="Version")
        self.labelVersion.grid(row=1, column=0, sticky=tk.W, padx=5, pady=5)

        self.cfgVersion = tk.StringVar()
        self.entryCfgVersion = tk.Entry(self.rightFrame, width=28, textvariable=self.cfgVersion, validate="focusout",
                                        validatecommand=self.version_changed_callback)
        self.entryCfgVersion.grid(row=1, columnspan=4, padx=5, pady=5)

        self.treeCfgElements = ttk.Treeview()
        self.init_right_frame()

        self.window.mainloop()

    def version_changed_callback(self):
        self.update_window_title("%s*" % self.cfg_file_name)
        return True

    def init_menu(self):
        menu = tk.Menu(self.window)

        self.window.config(menu=menu)

        file = tk.Menu(menu)
        file.add_command(label='New', command=self.new_config_file)
        file.add_command(label='Open', command=self.open_dialog)
        file.add_command(label='Save', command=self.save_config_file)
        file.add_command(label='Save as', command=self.save_as_config_file)
        menu.add_cascade(label='File', menu=file)
        file.add_command(label='Exit', command=self.window.quit)

    def remove_all_config_items(self):
        for i in self.treeCfgElements.get_children():
            self.treeCfgElements.delete(i)

    def new_config_file(self):
        self.cfg_file_name = DEFAULT_CFG_FILE_NAME
        self.update_window_title(self.cfg_file_name)
        self.remove_all_config_items()
        self.cfgVersion.set("")

    def get_cfg_elements(self):
        elements = list()

        for child in self.treeCfgElements.get_children():
            element_id = self.treeCfgElements.item(child)["values"][0]
            element_type = self.treeCfgElements.item(child)["values"][1]
            if self.get_input_type(element_type) == InputType.COMBO:
                element_size = self.cfg_def.get_type_size(element_type)
                enum_definition = self.cfg_def.get_enum(element_type)
                enum_item_name = self.treeCfgElements.item(child)["values"][2]
                element_value = enum_definition[enum_item_name]["val"]
            elif self.get_input_type(element_type) == InputType.CUSTOM:
                element_size = 0
                if element_type == "gpio_pin_t":
                    element_value = str(self.treeCfgElements.item(child)["values"][2]).zfill(4)
            else:
                element_size = 0
                if element_type == "string":
                    element_value = str(self.treeCfgElements.item(child)["values"][2])
                if element_type == "array" or element_type == "bd_address_t":
                    element_value = str(self.treeCfgElements.item(child)["values"][2])
                    ##### 17 because bd address is like:  xx:xx:xx:xx:xx:xx, mechanism to remove ':'
                    if element_type == "bd_address_t" and len(element_value) == 17:
                        element_value = element_value.replace(':','')
                else:
                    element_value = self.treeCfgElements.item(child)["values"][2]
            elements.append({"id": element_id, "type": element_type, "value": element_value, "size": element_size})

        return elements

    def save_config_file(self):
        f = open(self.cfg_full_file_name, mode='wb')
        if f is None:
            return
        self.config.set_version(self.cfgVersion.get())
        cfg_binary = self.config.get_binary(self.get_cfg_elements())
        f.write(cfg_binary)
        f.close()
        self.update_window_title(self.cfg_file_name)

    def save_as_config_file(self):
        f = filedialog.asksaveasfile(mode='wb', defaultextension=".img")
        if f is None:
            return
        self.cfg_file_name = os.path.basename(f.name)
        self.cfg_full_file_name = f.name
        self.update_window_title(self.cfg_file_name)
        self.config.set_version(self.cfgVersion.get())
        cfg_binary = self.config.get_binary(self.get_cfg_elements())
        f.write(cfg_binary)
        f.close()
        self.update_window_title(self.cfg_file_name)

    def init_right_frame(self):
        self.treeCfgElements = ttk.Treeview(self.rightFrame, columns=('Element', 'ID', 'Type'))
        self.treeCfgElements.heading('#0', text='Element name')
        self.treeCfgElements.heading('#1', text='ID')
        self.treeCfgElements.heading('#2', text='Type')
        self.treeCfgElements.heading('#3', text='Value')
        self.treeCfgElements.column('#0', stretch=tk.YES, width=250)
        self.treeCfgElements.column('#1', stretch=tk.YES, width=70)
        self.treeCfgElements.column('#2', stretch=tk.YES, width=150)
        self.treeCfgElements.column('#3', stretch=tk.YES, width=200)
        self.treeCfgElements.grid(row=4, columnspan=4, sticky=tk.NSEW)
        self.treeCfgElements.bind('<3>', self.config_context_menu)

        self.vsb = ttk.Scrollbar(self.rightFrame, orient="vertical", command=self.treeCfgElements.yview)
        self.vsb.grid(row=4, column=5, sticky=tk.NSEW)

        self.treeCfgElements.configure(yscrollcommand=self.vsb.set)

    def open_dialog(self):
        file_name = filedialog.askopenfilename(initialdir="./", title="Select a config file", filetypes=(("img files", "*.img"), ("all files", "*.*")))
        if file_name:
            f = open(file_name, mode='rb')
            if f is None:
                return
            cfg_binary = f.read()
            f.close()
            if self.config.set_binary(cfg_binary) is False:
                messagebox.showerror("Error", "Configuration file not parsed")
                return

            elements = self.config.get_elements()

            self.cfg_file_name = os.path.basename(file_name)
            self.cfg_full_file_name = file_name
            self.update_window_title(self.cfg_file_name)
            self.remove_all_config_items()
            self.cfgVersion.set(self.config.get_version())

            for el in elements:
                full_element_id_str = "0x%0.4X" % el['id']
                full_element_id = el['id'].to_bytes(2, byteorder="big")
                group_id = "0x%0.2X" % full_element_id[0]
                element_id = "0x%0.2X" % full_element_id[1]
                type_name = self.cfg_def.get_element_type_name(group_id, element_id)

                if self.get_input_type(type_name) == InputType.COMBO:
                    val = int.from_bytes(el['value'], "little")
                    val_name = self.cfg_def.get_enum_type_value_name(type_name, val)
                elif type_name == 'string':
                    val_name = el['value'].decode("utf-8")
                elif type_name == 'bd_address_t':
                    val_name = binascii.hexlify(el['value'][::-1]).decode(encoding='UTF-8')
                elif type_name == 'array':
                    val_name = binascii.hexlify(el['value']).decode(encoding='UTF-8')
                elif self.get_input_type(type_name) == InputType.CUSTOM:
                    if type_name == 'gpio_pin_t':
                        val = binascii.hexlify(el['value']).decode(encoding='UTF-8')
                        val_port = str(int(val[2:], 16))
                        val_pin = str(int(val[:2], 16))
                        val_name = "Port: [%s], Pin: [%s]" % (val_port, val_pin)
                else:
                    val_name = int.from_bytes(el['value'], "little")

                if type_name == "bd_address_t":
                    tmpval = val_name
                    tmpstr = ''
                    for i in range(0, len(val_name), 2):
                        if i != 10:
                            tmpstr = tmpstr + tmpval[i:i+2] + ':'
                        else:
                            tmpstr = tmpstr + tmpval[i:i+2]
                    val_name = tmpstr
                self.treeCfgElements.insert('', 'end', text=self.cfg_def.get_element_name(group_id, element_id),
                                            values=(full_element_id_str, type_name, val_name))

    def get_input_type(self, type_name):
        switcher = {
            "uint8_t": InputType.ENTRY,
            "int8_t": InputType.ENTRY,
            "uint16_t": InputType.ENTRY,
            "int16_t": InputType.ENTRY,
            "uint32_t": InputType.ENTRY,
            "int32_t": InputType.ENTRY,
            "string": InputType.ENTRY,
            "array": InputType.ENTRY,
            "bd_address_t": InputType.ENTRY,
            "gpio_pin_t": InputType.CUSTOM,
        }
        return switcher.get(type_name, InputType.COMBO)

    def on_group_changed(self, event):
        self.comboboxElementName['values'] = list(self.cfg_def.get_group_elements(self.groupName.get()).keys())
        self.comboboxElementName.current(0)
        self.on_element_changed(event)

    def on_element_changed(self, event):
        self.elementID = self.cfg_def.get_group_elements(self.groupName.get())[self.elementName.get()]
        # update type name label and units
        self.labelTypeName.config(text=self.cfg_def.get_type_name(self.groupName.get(), self.elementID))
        self.labelUnits.config(text=self.cfg_def.get_units(self.groupName.get(), self.elementID))
        # show correct widget for "value"
        self.show_input()

    def on_value_changed(self, event):
        print("value changed %s" % self.comboboxValue.get())

    def value_input_widget_init(self):
        self.comboboxValue = ttk.Combobox(self.leftFrame, textvariable=self.elementValue, width=25)
        self.comboboxValue.bind("<<ComboboxSelected>>", self.on_value_changed)
        self.comboboxValue1 = ttk.Combobox(self.leftFrame, textvariable=self.pinValue, width=3)
        self.comboboxValue1.bind("<<ComboboxSelected>>", self.on_value_changed)

        self.input_value = self.elementValue.get()

        def validate_input(var):
            new_value = var.get()

            if is_integer_type(self.cfg_def.get_type_name(self.groupName.get(), self.elementID)):
                try:
                    new_value == '' or int(new_value)
                    self.input_value = new_value
                except:
                    var.set(self.input_value)

            if is_array_type(self.cfg_def.get_type_name(self.groupName.get(), self.elementID)) or \
                    is_bd_address_type(self.cfg_def.get_type_name(self.groupName.get(), self.elementID)):
                try:
                    new_value == '' or hex(int(new_value, 16))[2:]
                    self.input_value = new_value
                except:
                    var.set(self.input_value)

        self.elementValue.trace('w', lambda nm, idx, mode, var=self.elementValue: validate_input(var))
        self.entryValue = tk.Entry(self.leftFrame, width=28, textvariable=self.elementValue)

        self.show_input()

    def restore_default_view(self):
        for w in (self.comboboxGroupNames, self.comboboxElementName, self.labelTypeName):
            w.grid(columnspan=1)
        self.comboboxValue.visible = False
        self.comboboxValue.config(width=25)
        self.comboboxValue1.visible = False
        self.comboboxValue1.grid_remove()
        if self.labelPortValue or self.labelPinValue:
            self.labelPortValue.grid_forget()
            self.labelPinValue.grid_forget()

    def show_input(self):
        widget = self.get_input_type(self.cfg_def.get_type_name(self.groupName.get(), self.elementID))
        defalut_val_present = self.cfg_def.has_default_value(self.groupName.get(), self.elementID)

        self.restore_default_view()

        if widget == InputType.COMBO:
            self.comboboxValue.visible = True
            self.entryValue.grid_forget()
            enum_dict = self.cfg_def.get_enum_values(self.groupName.get(), self.elementID)
            self.comboboxValue['values'] = list(enum_dict.keys())
            if defalut_val_present:
                default = self.cfg_def.get_default(self.groupName.get(), self.elementID)
                i = 0
                for k, v in enum_dict.items():
                    if v == default:
                        self.comboboxValue.current(i)
                        break
                    i += 1
            else:
                self.comboboxValue.current(0)
            self.comboboxValue.grid(row=4, column=1, sticky=tk.W, padx=5, pady=5)
        elif widget == InputType.CUSTOM:
            if self.groupName.get() == "GPIO FUNCTION":
                if self.cfg_def.get_type_name(self.groupName.get(), self.elementID) == "gpio_pin_t":
                    self.labelPortValue = tk.Label(self.leftFrame, text="Port")
                    self.labelPortValue.grid(row=4, column=1, sticky=tk.W)
                    self.labelPinValue = tk.Label(self.leftFrame, text="Pin")
                    self.labelPinValue.grid(row=4, column=3, sticky=tk.W)

                    self.comboboxValue.visible = True
                    self.entryValue.grid_forget()
                    self.comboboxValue['values'] = list(range(0, PORT_MAX_VALUE+1))
                    self.comboboxValue.current(0)
                    self.comboboxValue.config(width=3)
                    self.comboboxValue.grid(row=4, column=2, sticky=tk.W, padx=5, pady=5)

                    self.comboboxValue1.visible = True
                    self.comboboxValue1['values'] = list(range(0, PIN_MAX_VALUE+1))
                    self.comboboxValue1.current(0)
                    self.comboboxValue1.grid(row=4, column=4, sticky=tk.W, padx=5, pady=5)
                    for w in (self.comboboxGroupNames, self.comboboxElementName, self.labelTypeName):
                        w.grid(columnspan=4)
        else:
            self.comboboxValue.visible = False
            self.comboboxValue.grid_forget()
            self.entryValue.grid(row=4, column=1, sticky=tk.W, padx=5, pady=5)
            if defalut_val_present:
                default = self.cfg_def.get_default(self.groupName.get(), self.elementID)
                self.elementValue.set(default)
            else:
                self.elementValue.set("")

    def is_input_validate(self, value, type_name, group_id, element_id):
        if self.get_input_type(type_name) is InputType.ENTRY:
            if type_name == "string":
                max_size = self.cfg_def.get_max_size(group_id, element_id)
                if len(value) > max_size:
                    messagebox.showerror("Error", "String too long. Maximum allowed length is %d" % max_size)
                    return False
            elif type_name == "array":
                max_size = self.cfg_def.get_max_size(group_id, element_id)
                if (len(value) % 2) > 0:
                    messagebox.showerror("Error", "Byte array incomplete. One half-byte is missing")
                    return False
                if (len(value) / 2) > max_size:
                    messagebox.showerror("Error", "String too long. Maximum allowed length is %d" % max_size)
                    return False
            elif type_name == "bd_address_t":
                max_size = self.cfg_def.get_max_size(group_id, element_id)
                if (len(value) / 2) != max_size:
                    messagebox.showerror("Error", "String is incorrect. Allowed length is %d" % max_size)
                    return False
            elif is_integer_type(type_name):
                if self.cfg_def.is_range_present(group_id, element_id):
                    r_min, r_max = self.cfg_def.get_range(group_id, element_id)
                    val = int(value)
                    if val < int(r_min) or val > int(r_max):
                        messagebox.showerror("Error", "Value not in range <%s, %s>" % (r_min, r_max))
                        return False
        elif self.get_input_type(type_name) is InputType.CUSTOM:
            if is_gpio_pin_type(type_name):
                if not value.isdigit():
                    result = re.findall(r'\[(.*?)\]', value)
                    try:
                        port = result[0]
                        pin = result[1]
                        if not (port.isdigit() and pin.isdigit()):
                            messagebox.showerror("Error", "Value not int")
                            return False
                        if not (int(port) <= PORT_MAX_VALUE and int(pin) <= PIN_MAX_VALUE):
                            messagebox.showerror("Error", "Value not in range Port: <0, %s>, Pin: <0, %s>" % (PORT_MAX_VALUE, PIN_MAX_VALUE))
                            return False
                    except:
                        messagebox.showerror("Error", "Input string should be: Port: [<port_id>], Pin: [<pin_id>]\nOnly change values between brackets!")
                        return False

        return True

    def add_element_callback(self):
        type_name = self.cfg_def.get_type_name(self.groupName.get(), self.elementID)
        group_id = self.groupDict[self.groupName.get()]
        element_id = self.elementID
        if self.is_input_validate(self.elementValue.get(), type_name, group_id, element_id) is False:
            return

        full_element_id = pack("<BB", int(self.groupDict[self.groupName.get()], 0), int(self.elementID, 0))
        full_element_id_str = "0x%0.4X" % int.from_bytes(full_element_id, "big")

        element_type = self.cfg_def.get_type_name(self.groupName.get(), self.elementID)

        if element_type == "bd_address_t":
            tmpval = self.elementValue.get()
            tmpstr = ''
            for i in range(0, len(self.elementValue.get()),2):
                if i != 10:
                    tmpstr = tmpstr + tmpval[i:i+2] + ':'
                else:
                    tmpstr = tmpstr + tmpval[i:i+2]

            self.treeCfgElements.insert('', 'end', text=self.elementName.get(), values=(full_element_id_str, element_type, tmpstr))
        elif element_type == "gpio_pin_t":
            self.treeCfgElements.insert('', 'end',
                                        text=self.elementName.get(),
                                        values=(full_element_id_str,
                                                element_type,
                                                "Port: [" + self.elementValue.get() +
                                                "] ,Pin: [" + self.pinValue.get()+ "]"))
        else:
            self.treeCfgElements.insert('', 'end', text=self.elementName.get(),
                                        values=(full_element_id_str, element_type, self.elementValue.get()))
        self.update_window_title("%s*" % self.cfg_file_name)  # config change indication, '*' added

        # clean input
        if self.get_input_type(self.cfg_def.get_type_name(self.groupName.get(), self.elementID)) is InputType.ENTRY:
            self.elementValue.set("")
        elif self.get_input_type(self.cfg_def.get_type_name(self.groupName.get(), self.elementID)) is InputType.CUSTOM:
            if element_type == "gpio_pin_t":
                self.elementValue.set("0")
                self.pinValue.set("0")

    def config_context_menu(self, event):
        def delete_config_element():
            self.treeCfgElements.delete(rowid)
            self.update_window_title("%s*" % self.cfg_file_name)

        def edit_config_element():
            self.update_window_title("%s*" % self.cfg_file_name)  # config change indication, '*' added
            item = self.treeCfgElements.item(rowid)
            #if type(item['values'][2]) is int:
            #    if int(math.log10(item['values'][2]))+1 <12:
            #        item['values'][2] = str(item['values'][2]).zfill(12)
            #    else:
            #        item['values'][2] = str(item['values'][2])

            ### Remove ':' from bd_address_t before edit popup window
            if item['values'][1] == 'bd_address_t' and len(item['values'][2]) == 17:
                item['values'][2] = item['values'][2].replace(':','')

            self.edit_popup = edit_popup_window(self.window, item['text'], self.get_input_type(item['values'][1]),
                                                    item['values'][1], item['values'][2])
            self.window.wait_window(self.edit_popup.top)

            full_element_id = int(item['values'][0], 0)
            group_id, element_id = full_element_id.to_bytes(2, byteorder='big')
            element_id_str = "0x%0.2X" % element_id
            group_id_str = "0x%0.2X" % group_id
            if self.is_input_validate(self.edit_popup.edit_value.get(), item['values'][1], group_id_str, element_id_str) is False:
                return

            item['values'][2] = self.edit_popup.edit_value.get()
            ### Add ':' to bd_address_t to occur in the GUI window
            if item['values'][1] == "bd_address_t":
                tmpval = item['values'][2]
                tmpstr = ''
                for i in range(0, len(item['values'][2]), 2):
                    if i != 10:
                        tmpstr = tmpstr + tmpval[i:i + 2] + ':'  # ':'.join(myval[i:i+2])
                    else:
                        tmpstr = tmpstr + tmpval[i:i + 2]
                item['values'][2] = tmpstr

            self.treeCfgElements.item(rowid, text=item['text'], values=item['values'])

        # create a popup menu
        rowid = self.treeCfgElements.identify('item', event.x, event.y)
        if rowid:
            self.treeCfgElements.selection_set(rowid)
            self.treeCfgElements.focus_set()
            self.treeCfgElements.focus(rowid)

            menu = tk.Menu(self.window, tearoff=0)
            menu.add_command(label="Edit", command=edit_config_element)
            menu.add_command(label="Delete", command=delete_config_element)
            menu.post(event.x_root, event.y_root)
        else:
            pass


class edit_popup_window(object):
    def __init__(self, master, title, input_type, type_name, value):
        self.top = tk.Toplevel(master)
        self.top.wm_title(title)
        self.edit_value = tk.StringVar()
        self.edit_value.set(value)
        self.cfg_def = ConfigDefinition()

        self.input_value = self.edit_value.get()

        def validate_input(var):
            new_value = var.get()

            if is_integer_type(type_name):
                try:
                    new_value == '' or int(new_value)
                    self.input_value = new_value
                except:
                    var.set(self.input_value)

            # if is_array_type(self.cfg_def.get_type_name(self.groupName.get(), self.elementID)):
            if is_array_type(type_name) or is_bd_address_type(type_name):
                try:
                    new_value == '' or hex(int(new_value, 16))[2:]
                    self.input_value = new_value
                except:
                    var.set(self.input_value)

            if is_gpio_pin_type(type_name):
                var.set(new_value)

        if input_type == InputType.ENTRY or input_type == InputType.CUSTOM:
            self.edit_value.trace('w', lambda nm, idx, mode, var=self.edit_value: validate_input(var))
            self.e = tk.Entry(self.top, width=28, textvariable=self.edit_value)
        else:
            self.e = ttk.Combobox(self.top, textvariable=self.edit_value, width=28)
            enum_dict = self.cfg_def.get_enum(type_name)
            self.e['values'] = list(enum_dict.keys())
            self.e.current(self.e['values'].index(value))

        self.e.pack(pady=20, padx=70)
        self.b = tk.Button(self.top, text='OK', command=self.cleanup)
        self.top.bind('<Return>', self.cleanup)
        self.b.pack(pady=20)

    def cleanup(self, event=None):
        self.top.destroy()


apl = Application()
