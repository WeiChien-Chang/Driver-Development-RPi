# Driver-Development
- **Linux 驅動基本功**：`alloc_chrdev_region` / `cdev_add` / `class_create` / `device_create` 的生命週期管理，搭配 `copy_to_user` / `copy_from_user` 完成簡潔的 UAPI。  
- **GPIO 控制與資源管理**：`gpio_is_valid`、`gpio_request`、`gpio_direction_input/output`、`gpio_set/get_value`、`gpio_export`。  
- **穩定度**：模組卸載時完整釋放資源（GPIO/unexport、`cdev_del`、`unregister_chrdev_region`）。
