<?php

/**
 * @file
 * This file contains all theme and preprocess functions
 */

/**
 * Implements template_preprocess_HOOK().
 */
function islandora_basic_collection_preprocess_islandora_basic_collection_grid(&$variables) {
  islandora_basic_collection_preprocess_islandora_basic_collection($variables);
}

/**
 * Implements template_preprocess_HOOK().
 */
function islandora_basic_collection_preprocess_islandora_basic_collection_wrapper(&$variables) {
  $page_number = (empty($_GET['page'])) ? 0 : $_GET['page'];
  $page_size = (empty($_GET['pagesize'])) ? variable_get('islandora_basic_collection_page_size', '10') : $_GET['pagesize'];
  $islandora_object = $variables['islandora_object'];
  $results = islandora_basic_collection_get_objects($islandora_object, $page_number, $page_size);
  $total_count = count($results);
  pager_default_initialize($total_count, $page_size);
  $variables['collection_pager'] = theme('pager', array('quantity' => 10));
  $display = (empty($_GET['display'])) ? variable_get('islandora_basic_collection_default_view', 'grid') : $_GET['display'];
  $link_text = (empty($_GET['display'])) ? 'grid' : $_GET['display'];
  $query_params = drupal_get_query_parameters($_GET);
  global $base_url;

  $query_params['display'] = 'list';
  $list_link = array(
    'title' => t('List view'),
    'attributes' => array(
      'href' => url("islandora/object/$islandora_object->id", array('query' => $query_params)),
      'class' => array('islandora-view-list')
    ),
  );
  $query_params['display'] = 'grid';
  $grid_link = array(
    'title' => t('Grid view'),
    'attributes' => array(
      'href' => url("islandora/object/$islandora_object->id", array('query' => $query_params)),
      'class' => array('islandora-view-grid')
    ),
  );
  $collection_content = theme('islandora_basic_collection_grid', array('islandora_object' => $islandora_object, 'collection_results' => $results));

  if ($display == 'grid') {
    $grid_link['attributes']['class'][] = 'active';
  }
  else {
    $list_link['attributes']['class'][] = 'active';
  }

  $variables['view_links'] = array($grid_link, $list_link);
  $variables['collection_content'] = $collection_content;
}

/**
 * Implements template_preprocess_HOOK().
 */
function islandora_basic_collection_preprocess_islandora_basic_collection(&$variables) {
  global $base_url;
  global $base_path;
  $islandora_object = $variables['islandora_object'];

  try {
    $dc = $islandora_object['DC']->content;
    $dc_object = DublinCore::importFromXMLString($dc);
  }
  catch (Exception $e) {
    drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
  }
  $page_number = (empty($_GET['page'])) ? 0 : $_GET['page'];
  $page_size = (empty($_GET['pagesize'])) ? variable_get('islandora_basic_collection_page_size', '10') : $_GET['pagesize'];
  $results = $variables['collection_results'];
  $total_count = count($results);
  $variables['islandora_dublin_core'] = isset($dc_object) ? $dc_object : array();
  $variables['islandora_object_label'] = $islandora_object->label;
  $display = (empty($_GET['display'])) ? 'list' : $_GET['display'];
  if ($display == 'grid') {
    $variables['theme_hook_suggestions'][] = 'islandora_basic_collection_grid__' . str_replace(':', '_', $islandora_object->id);
  }
  else {
    $variables['theme_hook_suggestions'][] = 'islandora_basic_collection__' . str_replace(':', '_', $islandora_object->id);
  }
  if (isset($islandora_object['OBJ'])) {
    $full_size_url = url("islandora/object/{$islandora_object->id}/datastram/OBJ/view");
    $params = array(
      'title' => $islandora_object->label,
      'path' => $full_size_url);
    $variables['islandora_full_img'] = theme('image', $params);
  }
  if (isset($islandora_object['TN'])) {
    $full_size_url = url("islandora/objects/{$islandora_object->id}/datastream/TN/view");
    $params = array(
      'title' => $islandora_object->label,
      'path' => $full_size_url);
    $variables['islandora_thumbnail_img'] = theme('image', $params);
  }
  if (isset($islandora_object['MEDIUM_SIZE'])) {
    $full_size_url = url("islandora/object/{$islandora_object->id}/datastream/MEDIUM_SIZE/view");
    $params = array(
      'title' => $islandora_object->label,
      'path' => $full_size_url);
    $variables['islandora_medium_img'] = theme('params', $params);
  }

  $associated_objects_array = array();
  $start = $page_size * ($page_number);
  $end = min($start + $page_size, $total_count);

  for ($i = $start; $i < $end; $i++) {
    $pid = $results[$i]['object']['value'];
    $fc_object = islandora_object_load($pid);
    if (!isset($fc_object)) {
      // NULL object so don't show in collection view.
      continue;
    }
    $associated_objects_array[$pid]['object'] = $fc_object;
    try {
      $dc = $fc_object['DC']->content;
      $dc_object = DublinCore::importFromXMLString($dc);
      $associated_objects_array[$pid]['dc_array'] = $dc_object->asArray();
    }
    catch (Exception $e) {
      drupal_set_message(t('Error retrieving object %s %t', array('%s' => $islandora_object->id, '%t' => $e->getMessage())), 'error', FALSE);
    }
    $object_url = 'islandora/object/' . $pid;

    $title = $results[$i]['title']['value'];
    $associated_objects_array[$pid]['pid'] = $pid;
    $associated_objects_array[$pid]['path'] = $object_url;
    $associated_objects_array[$pid]['title'] = $title;
    $associated_objects_array[$pid]['class'] = drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $pid));
    if (isset($fc_object['TN'])) {
      $thumbnail_img = theme('image', array('path' => "$object_url/datastream/TN/view"));
    }
    else {
      $image_path = drupal_get_path('module', 'islandora');
      $thumbnail_img = theme('image', array('path' => "$image_path/images/folder.png"));
    }
    $associated_objects_array[$pid]['thumbnail'] = $thumbnail_img;
    $associated_objects_array[$pid]['title_link'] = l($title, $object_url, array('html' => TRUE, 'attributes' => array('title' => $title)));
    $associated_objects_array[$pid]['thumb_link'] = l($thumbnail_img, $object_url, array('html' => TRUE, 'attributes' => array('title' => $title)));
  }
  $variables['associated_objects_array'] = $associated_objects_array;
}

/**
 * Theme the collection policy management table.
 *
 * @param array $variables
 *   Variables passed to this theme function.
 */
function theme_islandora_basic_collection_policy_management_table(array $variables) {
  // Manually add the table select javascript as we are using a custom table.
  drupal_add_js('misc/tableselect.js');
  $table = $variables['table'];
  $row_elements = $table['rows'];
  $rows = array();
  foreach (element_children($row_elements) as $key) {
    $columns = array();
    $row_element = $row_elements[$key];
    foreach (element_children($row_element) as $key) {
      $column_element = $row_element[$key];
      $columns[] = array(
        'data' => drupal_render($column_element),
        'class' => isset($cell_element['#attributes']['class']) ? $column_element['#attributes']['class'] : NULL,
      );
    }
    $rows[] = $columns;
  }
  $variables = array(
    'header' => $table['#header'],
    'rows' => $rows,
    'attributes' => $table['#attributes'],
    'caption' => NULL,
    'colgroups' => NULL,
    'sticky' => NULL,
    'empty' => t("No Content Models"),
  );
  return theme('table', $variables);
}
