<?php
// $Id$

/**
 * @file
 * Admin forms callback file for the d7permissions module.
 */

/**
 * Menu callback: d7permissions_admin_perm.
 *
 * @see d7permissions_admin_perm_submit(), 
 * @see theme_d7permissions_admin_perm(), 
 */
function d7permissions_admin_perm($form_state, $rid = NULL) {
  if (is_numeric($rid)) {
    $result = db_query('SELECT r.rid, p.perm FROM {role} r LEFT JOIN {permission} p ON r.rid = p.rid WHERE r.rid = %d', $rid);
  }
  else {
    $result = db_query('SELECT r.rid, p.perm FROM {role} r LEFT JOIN {permission} p ON r.rid = p.rid ORDER BY name');
  }
  while ($role = db_fetch_object($result)) {
    $role_permissions[$role->rid] = $role->perm .',';
  }

  // Retrieve role names for columns
  $role_names = array();
  $results = db_query("SELECT role.rid AS role_id, role.name AS role_name, d7permissions_role_order.weight AS role_weight FROM {role} {role} INNER JOIN {d7permissions_role_order} ON role.rid = d7permissions_role_order.rid ORDER BY d7permissions_role_order.weight");
  while ($row = db_fetch_object($results)) {
    $role_names[$row->role_id] = $row->role_name;
  }

  // Render role/permission overview
  $options = array();
  foreach (module_list(FALSE, FALSE, TRUE) as $module) {
    if ($permissions = module_invoke($module, 'perm')) {
      $form['permission'][] = array(
        '#value' => $module,
      );
      asort($permissions);
      foreach ($permissions as $perm) {
        $options[$perm] = '';
        $form['permission'][$perm] = array('#value' => t($perm));
        foreach ($role_names as $rid => $name) {
          // Builds arrays for checked boxes for each role
          if (strpos($role_permissions[$rid], $perm .',') !== FALSE) {
            $status[$rid][] = $perm;
          }
        }
      }
    }
  }

  // Have to build checkboxes here after checkbox arrays are built
  foreach ($role_names as $rid => $name) {
    $form['checkboxes'][$rid] = array('#type' => 'checkboxes', '#options' => $options, '#default_value' => isset($status[$rid]) ? $status[$rid] : array());
    $form['role_names'][$rid] = array('#value' => $name, '#tree' => TRUE);
  }
  $form['submit'] = array('#type' => 'submit', '#value' => t('Save permissions'));

  return $form;
}

function d7permissions_admin_perm_submit($form, &$form_state) {
  // Save permissions
  $result = db_query('SELECT * FROM {role}');
  while ($role = db_fetch_object($result)) {
    if (isset($form_state['values'][$role->rid])) {
      db_query('DELETE FROM {permission} WHERE rid = %d', $role->rid);
      $form_state['values'][$role->rid] = array_filter($form_state['values'][$role->rid]);
      if (count($form_state['values'][$role->rid])) {
        db_query("INSERT INTO {permission} (rid, perm) VALUES (%d, '%s')", $role->rid, implode(', ', array_keys($form_state['values'][$role->rid])));
      }
    }
  }
  drupal_set_message(t('The changes have been saved.'));
  // Clear the cached pages
  cache_clear_all();
}

function theme_d7permissions_admin_perm($form) {
  drupal_add_js(drupal_get_path('module', 'd7permissions') . '/d7permissions.js');

  $roles = user_roles();
  foreach (element_children($form['permission']) as $key) {
    // Don't take form control structures
    if (is_array($form['permission'][$key])) {
      $row = array();
      // Module name
      if (is_numeric($key)) {
        $row[] = array('data' => t('@module module', array('@module' => drupal_render($form['permission'][$key]))), 'class' => 'module', 'id' => 'module-'. $form['permission'][$key]['#value'], 'colspan' => count($form['role_names']) + 1);
      }
      else {
        $row[] = array('data' => drupal_render($form['permission'][$key]), 'class' => 'permission');
        foreach (element_children($form['checkboxes']) as $rid) {
          if (is_array($form['checkboxes'][$rid])) {
            $row[] = array('data' => drupal_render($form['checkboxes'][$rid][$key]), 'class' => 'checkbox', 'title' => $roles[$rid] .' : '. t($key));
          }
        }
      }
      $rows[] = $row;
    }
  }
  $header[] = (t('Permission'));
  foreach (element_children($form['role_names']) as $rid) {
    if (is_array($form['role_names'][$rid])) {
      $header[] = array('data' => drupal_render($form['role_names'][$rid]), 'class' => 'checkbox');
    }
  }
  $output = theme('table', $header, $rows, array('id' => 'permissions'));
  $output .= drupal_render($form);
  return $output;
}

/**
 * Menu callback: d7permissions_role_order.
 *
 * @see d7permissions_role_order_submit(),
 * @see theme_d7permissions_role_order(),
 */
function d7permissions_role_order($form_state) {
  // Select role weights
  $results = db_query("SELECT role.rid AS role_id, role.name AS role_name, d7permissions_role_order.weight AS role_weight FROM {role} {role} INNER JOIN {d7permissions_role_order} ON role.rid = d7permissions_role_order.rid ORDER BY role_weight ASC");

  // Build the form
  $form['#attributes'] = array('name' => 'role-order-form');
  while ($row = db_fetch_object($results)) {
    $form['rows'][$row->role_id]['data'] = array(
      '#type' => 'value',
      '#value' => array($row->role_name),
    );
    $form['rows'][$row->role_id]['weight-' . $row->role_id] = array(
      '#type' => 'weight',
      '#default_value' => $row->role_weight,
      '#attributes' => array('class' => 'weight'),
    );
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save changes'),
  );
  return $form;
}

function d7permissions_role_order_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if (drupal_substr($key, 0, 6) == 'weight') {
      $id = str_replace('weight-', '', $key);
      db_query("UPDATE {d7permissions_role_order} SET weight=%d WHERE rid=%d", $value, $id);
    }
  }
  drupal_set_message(t('The changes have been saved.'));
  // Clear the cached pages
  cache_clear_all();
}

function theme_d7permissions_role_order($form) {
  // Loop throuth form rows and build our new table rows
  foreach ($form['rows'] as $id => $row) {
    if (intval($id)) {
      $this_row = $row['data']['#value'];
      $this_row[] = drupal_render($form['rows'][$id]['weight-' . $id]);
      $table_rows[] = array('data' => $this_row, 'class' => 'draggable');
    }
  }

  $header = array(t('Name'), t('Weight'));

  // Theme table	
  $output = t('Set your Roles in the order that you would like them to appear on the permissions page');
  $output .= theme('table', $header, $table_rows, array('id' => 'role-order-table'));
  $output .= drupal_render($form);

  // Convert table-based weight form to a draggable jQuery weight form
  drupal_add_tabledrag('role-order-table', 'order', 'sibling', 'weight', NULL, NULL, FALSE);

  return $output;
}