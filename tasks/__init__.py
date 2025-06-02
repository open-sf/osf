from invoke import Collection

from . import task

ns = Collection()
ns.add_task(task.package)
