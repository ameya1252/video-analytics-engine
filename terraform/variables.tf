variable "region" { type = string }
variable "cluster_name" { type = string }
variable "node_group_desired" { type = number default = 3 }
variable "gpu_node_group_desired" { type = number default = 1 }
