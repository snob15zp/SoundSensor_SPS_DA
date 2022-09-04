import sys
import os
try:
    import tkinter as tk
    isGUI = True
except ImportError:
    isGUI = False


def resource_path(relative_path):
    """"Get absolute path to resource, works for dev and for PyInstaller"""
    if getattr(sys, 'frozen', False):
        # we are running in a bundle
        bundle_dir = sys._MEIPASS
    else:
        # we are running in a normal Python environment
        bundle_dir = os.path.dirname(os.path.abspath(relative_path))

    return bundle_dir

def guiSelect(items_list, title, msg):
    root = tk.Tk()
    root.title(title)
    icon_path = resource_path("Dialog_D-Symbol_Color_RGB_Small_icon.ico") + "\\" + "Dialog_D-Symbol_Color_RGB_Small_icon.ico"
    root.iconbitmap(icon_path)
    root.bind('<Escape>', lambda x: root.destroy())
    tk.selected_item = -1
    tk.Label(root, text=msg, justify=tk.CENTER).pack(padx=10, pady=10)

    listbox = tk.Listbox(root)
    listbox.pack(padx=10, pady=10, expand=True, fill=tk.BOTH)
    listbox.focus_set()

    for item in items_list:
        listbox.insert(tk.END, ' %s | %s' % (item[0], item[1]))
    listbox.selection_set(0)

    def ok():
        tk.selected_item = listbox.index(tk.ACTIVE)
        root.destroy()

    ok_button = tk.Button(text='OK', command=ok)
    ok_button.pack(side=tk.RIGHT, padx=5, pady=5)
    ok_button.bind('<Return>', lambda x: ok_button.invoke())
    cancel_button = tk.Button(text='Cancel', command=lambda root=root: root.destroy())
    cancel_button.pack(side=tk.RIGHT, padx=5, pady=5)
    cancel_button.bind('<Return>', lambda x: root.destroy())

    listbox.bind('<Double-1>', lambda x: ok_button.invoke())
    listbox.bind('<Return>', lambda x: ok_button.invoke())
    listbox.bind('<space>', lambda x: ok_button.invoke())

    root.update()
    root.minsize(root.winfo_width(), root.winfo_height())
    tk.mainloop()

    return tk.selected_item


def consoleSelect(items_list, title, msg):
    print('+=========================')
    print('|      ', title)
    print('+-------------------------')
    print('|  ', msg)

    while True:
        index = 0
        print('+----+--------------------')
        for key, value , id in items_list:
            index += 1
            print('| %2d | %s | %s' % (index, key, value))
        print('+=========================')
        user_input = input('selection: ').strip()
        try:
            selected_item = int(user_input, 0)-1
            if (selected_item >= 0) and (selected_item < len(items_list)):
                return selected_item
            print('Selected index out of range, try again')
        except ValueError:
            print('Please enter an index of selected item')


def uiSelect(items_list, title='Selection', msg='Select itme'):
    if isGUI:
        return guiSelect(items_list, title, msg)
    else:
        return consoleSelect(items_list, title, msg)
