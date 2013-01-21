<?php

/**
 * @file
 * Important administration functions for the BSE Quotes module.
 */

function bse_quotes_config_form() {
	$form['bse_quote_code'] = array(
			'#type' => 'textfield',
			'#title' => 'Stock Code',
			'#description' => 'Type stock name and select your desired stock. It will be automatically converted into stock code.',
			'#autocomplete_path' => 'bse_quotes_names/autocomplete',
			'#required' => TRUE
	);
	$form['bse_quote_weight'] = array(
			'#type' => 'weight',
			'#title' => 'Weight',
			'#description' => 'Weight in listing',
			'#required' => TRUE
	);
	$form['bse_quote_config_form_submit'] = array(
		'#type' => 'submit',
		'#value' => 'Submit',
	);
	return $form;
}

function _bse_quote_names_autocomplete($string) {
	$stock_mapping = _get_stock_quotes_mapping_array();
	$matches = array();
	foreach($stock_mapping as $stock_code => $stock_name) {
		if(strpos($stock_name, $string) !== false) {
			$matches[$stock_code] = $stock_name;
		}
	}
	drupal_json_output($matches);
}

function bse_quotes_config_form_validate($form_id,&$form_values) {
	$stock_mapping = _get_stock_quotes_mapping_array();
	if(!array_key_exists($form_values['values']['bse_quote_code'], $stock_mapping)) {
		form_set_error('bse_quote_code', t('This stock code does not exist. Please select stock from autocomplete options only.'));
	}
	
	/* Check if code does not exist */
	$query = db_select('bse_quotes', 'bse');
	$query
	->condition('bse.stock_code', $form_values['values']['bse_quote_code'], '=')
	->fields('bse', array('id'))
	->range(0, 5);
	$result = $query->execute();
	if($result->rowCount() > 0) {
		form_set_error('bse_quote_code', t('This stock code already configured to display.'));
	}
}

function _get_stock_quotes_mapping_array() {
	$my_data = &drupal_static(__FUNCTION__);
	
	if (!isset($my_data)) {
		if ($cache = cache_get('stock_mapping')) {
			$stock_mapping = $cache->data;
		}
		else {
			$stock_codes_file = drupal_get_path('module', 'bse_quotes') . '/stockcode.txt';
			$handle = fopen($stock_codes_file, "r");
			$stock_mapping = fread($handle, filesize($stock_codes_file));
			fclose($handle);
			cache_set('stock_mapping', $stock_mapping, 'cache');
		}
	}
	return unserialize($stock_mapping);
}

function bse_quotes_config_form_submit($form_id,&$form_values) {
	
	$stock_mapping = _get_stock_quotes_mapping_array();
	
	$bse_quote_id = db_insert('bse_quotes')
	->fields(array(
			'stock_code' => $form_values['values']['bse_quote_code'],
			'stock_name' => $stock_mapping[$form_values['values']['bse_quote_code']],
			'status' => 1,
	))
	->execute();
	
	drupal_set_message(t('Stock added.'),'status');
}


function bse_stocks_list() {
	//Create a list of headers for your Html table (see Drupal 7 docs for theme_table here
	$header = array(
			'nid' => t('title'),
			'title' => t('nid'),
			'type' => t('type'),
			'created' => t('created'),
			'status' => t('status'),
	);
	
	//Create the Sql query. This uses various parts of Drupal 7's new DBTNG database abstraction layer.
	
	$query = db_select('node', 'n')
	->condition('status', 1) //Only published nodes, change condition as it suits you
	->extend('PagerDefault')  //Pager Extender
	->limit(2)    //2 results per page
	->extend('TableSort')  //Sorting Extender
	->orderByHeader($header)//Field to sort on is picked from $header
	->fields('n', array(
			'nid',
			'title',
			'type',
			'created',
			'status',
	));
	
	$results = $query
	->execute();
	
	$rows = array();
	
	foreach ($results as $node) {
		$rows[$node->nid] = array(
				'title' => l($node->title, 'node/' . $node->nid),
				'nid' => $node->nid,
				'type' => $node->type,
				'created' => format_date($node->created),
				'status' => $node->status,
				//'#attributes' => array('class' => array('article-row')),
		);
	}
	$form['registration_table'] = array(
			'#type' => 'tableselect',
			'#header' => $header,
			'#options' => $rows,
			//'#js_select' => FALSE,
			//'#multiple' => TRUE,
			'#prefix' => '<table><tr><td>',
			'#suffix' => '</td></tr>',
			'#attributes' => array('class' => array('article-row')),
	);
	$form['submit'] = array('#type' => 'submit',
			'#value' => t('Register'),
			'#disabled' => FALSE,
			'#prefix' => '<tr><td>',
			'#suffix' => '</td></tr></table>');
	$form['pager'] = array('#markup' => theme('pager'));
	//pr($form);
	return $form;
}