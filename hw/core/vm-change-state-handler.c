/*
 *  qdev vm change state handlers
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License,
 *  or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "hw/qdev-core.h"
#include "sysemu/runstate.h"

static int qdev_get_dev_tree_depth(DeviceState *dev)
{
    int depth;

    for (depth = 0; dev; depth++) {
        BusState *bus = dev->parent_bus;

        if (!bus) {
            break;
        }

        dev = bus->parent;
    }

    return depth;
}

VMChangeStateEntry *qdev_add_vm_change_state_handler(DeviceState *dev,
                                                     VMChangeStateHandler *cb,
                                                     void *opaque)
{
    return qdev_add_vm_change_state_handler_full(dev, cb, NULL, opaque);
}

/**
 * qdev_add_vm_change_state_handler_full:
 * @dev: the device that owns this handler
 * @cb: the callback function to be invoked
 * @pre_change_cb: a pre VM state change callback function to be invoked
 * @opaque: user data passed to the callback function
 *
 * This function uses qemu_add_vm_change_state_handler_prio_full() to invoke
 * callbacks in qdev tree depth order.  Ordering is desirable when callbacks of
 * children depend on their parent's callback having completed first.
 *
 * For example, when qdev_add_vm_change_state_handler_full() is used, a host
 * controller's callback is invoked before the children on its bus when the VM
 * starts running.  The order is reversed when the VM stops running.
 *
 * Returns: an entry to be freed with qemu_del_vm_change_state_handler()
 */
VMChangeStateEntry *qdev_add_vm_change_state_handler_full(
    DeviceState *dev, VMChangeStateHandler *cb,
    VMChangeStateHandler *pre_change_cb, void *opaque)
{
    int depth = qdev_get_dev_tree_depth(dev);

    return qemu_add_vm_change_state_handler_prio_full(cb, pre_change_cb, opaque,
                                                      depth);
}
