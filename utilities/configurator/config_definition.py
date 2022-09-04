import xml.etree.ElementTree as eT
import common as common


class ConfigDefinition:
    def __init__(self):
        try:
            self.xml = eT.parse('config-spec.xml').getroot()
        except eT.ParseError:
            raise Exception('Config definition', 'XML not well-formed')

    def get_type_size(self, type_name):
        enum_tag = self.xml.find("enumTypes/enum[@type='%s']" % type_name)
        return int(enum_tag.get('size'))

    def get_groups(self):
        groups = dict()
        for group in self.xml.findall('parameterGroups/group'):
            name = group.get('name')
            groups[name] = group.get('id')

        return groups

    def get_group_elements(self, group_name):
        elements = dict()
        group_tag = self.xml.find("parameterGroups/group[@name='%s']" % group_name)

        for el in group_tag.findall('./element'):
            name = el.find('name').text
            elements[name] = el.get('id')

        return elements

    def get_enum_values(self, group_name, element_id):
        values = dict()
        type_name = self.get_type_name(group_name, element_id)
        enum_tag = self.xml.find("enumTypes/enum[@type='%s']" % type_name)

        for item in enum_tag.findall('./item'):
            name = item.get('name')
            values[name] = item.text

        return values

    def get_enum(self, type_name):
        values = dict()
        enum_tag = self.xml.find("enumTypes/enum[@type='%s']" % type_name)

        for item in enum_tag.findall('./item'):
            name = item.get('name')
            val = item.get('value')
            values[name] = {"txt": item.text, "val": int(val, 0)}

        return values

    def get_type_name(self, group_name, element_id):
        element_tag = self.xml.find("parameterGroups/group[@name='%s']/element[@id='%s']" % (group_name, element_id))
        type_name = element_tag.find('type').text
        return type_name

    def get_max_size(self, group_id, element_id):
        element_tag = self.xml.find("parameterGroups/group[@id='%s']/element[@id='%s']" % (group_id, element_id))
        try:
            max_size = int(element_tag.find('max_size').text)
        except AttributeError:
            max_size = 0
        return max_size

    def get_units(self, group_name, element_id):
        element_tag = self.xml.find("parameterGroups/group[@name='%s']/element[@id='%s']" % (group_name, element_id))
        try:
            units = element_tag.find('units').text
        except AttributeError:
            units = ""
        return units

    def get_default(self, group_name, element_id):
        element_tag = self.xml.find("parameterGroups/group[@name='%s']/element[@id='%s']" % (group_name, element_id))
        try:
            units = element_tag.find('default').text
        except AttributeError:
            units = ""
        return units

    def has_default_value(self, group_name, element_id):
        element_tag = self.xml.find("parameterGroups/group[@name='%s']/element[@id='%s']" % (group_name, element_id))
        try:
            if element_tag.find('default').text != "":
                default_present = True
        except AttributeError:
            default_present = False
        return default_present

    def is_range_present(self, group_id, element_id):
        element_tag = self.xml.find("parameterGroups/group[@id='%s']/element[@id='%s']" % (group_id, element_id))
        try:
            if element_tag.find('min').text != "" and element_tag.find('max').text != "":
                range_present = True
        except AttributeError:
            range_present = False
        return range_present

    def get_range(self, group_id, element_id):
        element_tag = self.xml.find("parameterGroups/group[@id='%s']/element[@id='%s']" % (group_id, element_id))
        try:
            minimum = element_tag.find('min').text
            maximum = element_tag.find('max').text
        except AttributeError:
            minimum = 0
            maximum = 0
        return minimum, maximum

    def get_group_name(self, group_id):
        group_tag = self.xml.find("parameterGroups/group[@id='%s']" % group_id)
        return group_tag.get('name')

    def get_element_name(self, group_id, element_id):
        element_tag = self.xml.find("parameterGroups/group[@id='%s']/element[@id='%s']" % (group_id, element_id))
        return element_tag.find('name').text

    def get_element_type_name(self, group_id, element_id):
        element_tag = self.xml.find("parameterGroups/group[@id='%s']/element[@id='%s']" % (group_id, element_id))
        type_name = element_tag.find('type').text
        return type_name

    def get_enum_type_value_name(self, type_name, val):
        element_tag = self.xml.find("enumTypes/enum[@type='%s']/item[@value='0x%0.2X']" % (type_name, val))
        return element_tag.get('name')

