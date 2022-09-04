import inspect
import os

class verbose_mask_t(set):
    def __init__(self):
        self.modules = ['__main__', 'link', 'jlink', 'message']
#        self.levels = ['debug', 'info']
#        for l in self.levels:
#           self.add_tag(l)

    def add_tag(self, postfix):
        self.update([ '%s_%s' % (x, postfix) for x in self.modules])

    def sub_tag(self, postfix):
        self.difference_update(['%s_%s' % (x, postfix) for x in self.modules])


verbose_mask = verbose_mask_t()


def verbose_mask_add_info():
    verbose_mask.add_tag('info')


def verbose_mask_add_debug():
    verbose_mask.add_tag('debug')


def verbose_mask_sub_info():
    verbose_mask.sub_tag('info')


def verbose_mask_sub_debug():
    verbose_mask.sub_tag('debug')


def verbose_mask_print():
    print(verbose_mask)


def cli_print(level, *args, **kwargs):
    frame_info_filename = inspect.getframeinfo(inspect.stack()[2][0]).filename
    mask = os.path.basename(frame_info_filename).split(".")[0] + level
    if mask in verbose_mask:
        print(*args, **kwargs)


def cli_info(*args, **kwargs):
    cli_print('_info', *args, **kwargs)


def cli_debug(*args, **kwargs):
    cli_print('_debug', *args, **kwargs)


def cli_msg(*args, **kwargs):
    print(*args, **kwargs)
